# docker build -t autosca-tool:0.1 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM python:3-alpine
COPY Pipfile feature_extraction classification_model analyze_dataset.sh /autosca-tool/
WORKDIR /autosca-tool
RUN set -x && apk update
RUN apk add --no-cache bash
RUN python3 -m pip install --upgrade pip
RUN pip3 install --upgrade pip setuptools
RUN pip3 install pipenv
RUN pipenv install --system --deploy --ignore-pipfile
ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
