# docker build -t autosca-tool:0.1 -f docker_containers/Dockerfile .
# docker run --entrypoint "/bin/bash" -it autosca-tool:0.1
FROM amancevice/pandas
COPY docker_containers/install_packages.sh /autosca-tool/
WORKDIR /autosca-tool
RUN ["chmod", "+x", "install_packages.sh"]
RUN ./install_packages.sh
COPY requirements.txt feature_extraction classification_model analyze_dataset.sh /autosca-tool/
RUN pip install pipenv
# pipenv lock --keep-outdated --requirements > requirements.txt
RUN pip install -r requirements.txt
# Run pipenv once so that the virtual environment is created
RUN pipenv run python3 --version

ENTRYPOINT ["bash", "/autosca-tool/analyze_dataset.sh"]
