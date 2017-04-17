FROM ubuntu

RUN apt-get update
RUN apt-get install -y python3

ENV WWW_DATA /srv/www

# Copy html
RUN mkdir -p ${WWW_DATA}
WORKDIR ${WWW_DATA}

# Run http server on port 8080
EXPOSE  8080
CMD ["python3", "-m", "http.server", "8080"]

