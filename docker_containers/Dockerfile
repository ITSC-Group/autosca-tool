# docker build -t autosca-tool:0.1 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM python:3-alpine
COPY requirements.txt feature_extraction classification_model analyze_dataset.sh /autosca-tool/
WORKDIR /autosca-tool
RUN echo "http://dl-cdn.alpinelinux.org/alpine/edge/community" >> /etc/apk/repositories
# Add HDF5 support
RUN apk add --no-cache --allow-untrusted --repository http://dl-3.alpinelinux.org/alpine/edge/testing hdf5 hdf5-dev
RUN apk --no-cache --update-cache add gcc gfortran py-pip build-base wget freetype-dev libpng-dev openblas-dev
RUN ln -s /usr/include/locale.h /usr/include/xlocale.h
# pipenv lock --keep-outdated --requirements > requirements.txt
RUN pip install -r requirements.txt
ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
