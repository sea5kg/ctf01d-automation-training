FROM sea5kg/ctf01d-automation-training-build-environment:2026-04-30

WORKDIR /root/

COPY . /root/ctf01d-automation-training
WORKDIR /root/ctf01d-automation-training
RUN ./build_simple.sh

FROM sea5kg/ctf01d-automation-training-release-environment:2026-04-30

LABEL "maintainer"="Evgenii Sopov <mrseakg@gmail.com>"
LABEL "repository"="https://github.com/sea5kg/ctf01d-automation-training"

WORKDIR /app/
RUN mkdir -p /app

COPY --from=0 /root/ctf01d-automation-training/ctf01d-automation-training /usr/bin/ctf01d-automation-training
COPY ./data /app/data
COPY ./www /app/www

EXPOSE 10551
CMD ctf01d-automation-training
