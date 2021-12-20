# docker build -t autosca-tool:0.1 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM python:3-alpine
ADD . /autosca-tool
WORKDIR /autosca-tool
RUN apk add --no-cache bash
RUN pip install pipenv
RUN pipenv install --system --deploy --ignore-pipfile
ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
