FROM eclipse-temurin:17-jre-alpine
WORKDIR /app
# 빌드된 jar 파일을 컨테이너 내부로 복사
COPY build/libs/*.jar app.jar
EXPOSE 8080
ENTRYPOINT ["java", "-jar", "app.jar"]