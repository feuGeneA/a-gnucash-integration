# A GnuCash Integration

A minimum viable GnuCash integration using a PostgreSQL backend.

## Building and Running

Configured via `Dockerfile` to build and run in a Debian container.

1.   Build this code with `docker build -t a-gnucash-integration .`
2.   Run this code with `docker run --net=host a-gnucash-integration`
