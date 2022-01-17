# docker build -t itscgroup/autosca-analysis:0.7 -f docker_containers/Dockerfile .
# docker push itscgroup/autosca-analysis:0.7
# docker tag itscgroup/autosca-analysis:0.7 itscgroup/autosca-analysis:latest
# docker push itscgroup/autosca-analysis:latest
# docker run -it --mount type=bind,source=/home/datasets,target=/home/datasets itscgroup/autosca-analysis:latest --folder /home/datasets/2021-12-20-damnvulnerableopenssl
# docker run --mount type=bind,source=/home/datasets,target=/home/datasets --entrypoint "/bin/bash" -it itscgroup/autosca-analysis:latest
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
