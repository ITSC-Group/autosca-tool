# autosca-tool:0.1
FROM python:3
ADD . /autosca-tool
WORKDIR /autosca-tool
RUN pipenv install
ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
