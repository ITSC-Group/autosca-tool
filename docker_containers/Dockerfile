# docker build -t autosca-tool:0.4 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM amancevice/pandas
COPY docker_containers/install_packages.sh /autosca-tool/
WORKDIR /autosca-tool
RUN ["chmod", "+x", "install_packages.sh"]
RUN ./install_packages.sh
RUN pip install pipenv
COPY requirements.txt /autosca-tool/
# pipenv lock --keep-outdated --requirements > requirements.txt
RUN pipenv install -r requirements.txt
# Run pipenv once so that the virtual environment is created
RUN pipenv run python3 --version
# Add the actual autosca-tool source code (https://stackoverflow.com/q/26504846/4091513)
COPY feature_extraction /autosca-tool/feature_extraction
COPY classification_model /autosca-tool/classification_model
COPY analyze_dataset.sh /autosca-tool/

ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
