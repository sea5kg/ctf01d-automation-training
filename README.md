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

Build image

```
docker build -t sea5kg/ctf01d-automation-training:2026-04-30 -f Dockerfile .
```

```
docker run --rm -p 10551:10551 -v $PWD/data:$PWD/app/data sea5kg/ctf01d-automation-training:2026-04-30
```

How web will be available on http://localhost:10551/

## Authors

* Evgenii Sopov (mrseakg@gmail.com)
