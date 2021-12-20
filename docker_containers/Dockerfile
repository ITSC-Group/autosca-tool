# docker build -t autosca-tool:0.1 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM python:3-slim
COPY requirements.txt feature_extraction classification_model analyze_dataset.sh /autosca-tool/
WORKDIR /autosca-tool
RUN python3 -m pip install --upgrade pip setuptools
# pipenv lock --keep-outdated --requirements > requirements.txt
RUN pip install -r requirements.txt
ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
