#include "tempolink/juce/network/http/news/NewsApiClient.h"

#include <thread>
#include <utility>

#include <juce_events/juce_events.h>

#include "tempolink/juce/constants/ApiPaths.h"
#include "tempolink/juce/network/http/common/HttpTransport.h"
#include "tempolink/juce/network/http/common/JsonParse.h"

namespace {

namespace http = tempolink::juceapp::network::http;
namespace jsonparse = tempolink::juceapp::jsonparse;

std::vector<NewsItem> ParseNewsList(const juce::String& text) {
  std::vector<NewsItem> items;
  const auto json = jsonparse::Parse(text);
  const auto* list = &json;
  if (json.is_object()) {
    if (const auto it = json.find("notices"); it != json.end() && it->is_array()) {
      list = &(*it);
    } else if (const auto it = json.find("items");
               it != json.end() && it->is_array()) {
      list = &(*it);
    } else if (const auto it = json.find("content");
               it != json.end() && it->is_array()) {
      list = &(*it);
    }
  }
  if (!list->is_array()) {
    return items;
  }

  items.reserve(list->size());
  for (const auto& item : *list) {
    if (!item.is_object()) {
      continue;
    }
    NewsItem news;
    news.title = jsonparse::String(item, "title", "");
    news.body = jsonparse::String(item, "body",
                                  jsonparse::String(item, "summary", ""));
    news.published_at = jsonparse::String(item, "publishedAt", "");
    if (news.title.isNotEmpty() || news.body.isNotEmpty()) {
      items.push_back(std::move(news));
    }
  }
  return items;
}

}  // namespace

NewsApiClient::NewsApiClient(std::string base_url)
    : base_url_(http::NormalizeBaseUrl(std::move(base_url))) {}

void NewsApiClient::fetchNewsAsync(Callback callback) const {
  std::thread([base_url = base_url_, callback = std::move(callback)]() mutable {
    juce::String error_text;
    std::vector<NewsItem> items;

    http::Request request;
    request.method = "GET";
    request.path = tempolink::juceapp::constants::kNewsPath;
    request.timeout_ms = 3500;

    const auto response = http::Perform(base_url, request);
    if (!response.transport_ok) {
      error_text = "News API call failed.";
    } else if (response.isHttpSuccess()) {
      items = ParseNewsList(response.body);
    } else {
      error_text = "News API HTTP " + juce::String(response.status_code);
    }

    juce::MessageManager::callAsync([callback = std::move(callback), items = std::move(items),
                                     error_text]() mutable {
      callback(error_text.isEmpty(), std::move(items), error_text);
    });
  }).detach();
}
