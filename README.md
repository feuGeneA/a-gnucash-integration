# A GnuCash Integration

A minimum viable GnuCash integration using a PostgreSQL backend.

## Building and Running

Configured via `Dockerfile` to build and run in a Debian container.

### On Linux

1.   Start a postgres server at `postgres://gnc:gnc@127.0.0.1:5432/gnucash`, such as with `docker-compose -f docker-compose-postgres.yml up` using the file in this directory.  (But don't do it IN this directory, otherwise the local-directory volume it creates will pollute the build context for the `Dockerfile` in this directory; copy it to another directory and run it there.)
2.   Build this code with `docker build -t a-gnucash-integration .`
3.   Run this code with `docker run --net=host a-gnucash-integration`

### On other platforms

The `--net=host` argument to `docker run` apparently only works on Linux.  To get this code to run on other platforms, you'd have to manually satisfy the build dependencies that are specified in `Dockerfile`
