FROM python:3.12-slim
# This sets python environment for the image

# Installs gcc compiler & standard C headers
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    libc6-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy project files into the image
COPY travel.c .
COPY server.py .
COPY index.html .

# Compile travel.c into /usr/local/bin/travel
RUN gcc -Wall -Wextra -Werror travel.c -lm -o /usr/local/bin/travel

# Port 8000
EXPOSE 8000

# This sets it to the main program run when the container starts
# Passes arguments to docker run directly to travel file (binary)
ENTRYPOINT ["/usr/local/bin/travel"]
