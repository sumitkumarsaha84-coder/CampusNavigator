FROM gcc:13

RUN apt-get update && apt-get install -y --no-install-recommends cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel 2

EXPOSE 8080

CMD ["sh", "-c", "exec ./build/CampusNavigator --web --port \"${PORT:-8080}\" --webroot /app/web"]
