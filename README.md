# ctf01d-automation-training

## Use Docker

### Build docker images with environment

Build environment (change the date)

```
docker build -t sea5kg/ctf01d-automation-training-build-environment:2026-04-30 -f Dockerfile.build-environment .
```

Release environment (change the date)

```
docker build -t sea5kg/ctf01d-automation-training-release-environment:2026-04-30 -f Dockerfile.release-environment .
```

## Authors

* Evgenii Sopov (mrseakg@gmail.com)
