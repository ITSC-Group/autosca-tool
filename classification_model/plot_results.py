import argparse
import logging
import os
import pickle
import warnings

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from sklearn.utils import check_random_state

from classification_model.result_directories import ResultDirectories
from pycsca.constants import *
from pycsca.csv_reader import CSVReader
from pycsca.plot_utils import classwise_barplot_for_dataset, \
    bar_grid_for_dataset, plot_learning_curves_importances
from pycsca.utils import setup_logging, create_dir_recursively

if __name__ == "__main__":
    warnings.simplefilter("ignore")
    warnings.simplefilter('always', category=UserWarning)
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--folder', required=True,
                        help='Folder that contains the input files Packets.pcap and Client Requests.csv '
                             'and that the output files will be written to')
    args = parser.parse_args()
    folder = args.folder

    result_dirs = ResultDirectories(folder=folder)
    if os.path.exists(result_dirs.accuracies_file):
        with open(result_dirs.accuracies_file, 'rb') as f:
            metrics_dictionary = pickle.load(f)
        f.close()
    else:
        raise ValueError("The learning simulations are not done yet")
    cv_iterations_dict = metrics_dictionary[CV_ITERATIONS_LABEL]
    setup_logging(log_path=result_dirs.plotting_log_file)
    logger = logging.getLogger("Plotting")
    logger.info("Arguments {}".format(args))

    csv_reader = CSVReader(folder=folder, seed=42)
    dataset = args.folder.split('/')[-1]
    random_state = check_random_state(42)

    figsize = (7, 5)
    sfigsize = (4, 4)

    with open(result_dirs.vulnerable_file, 'rb') as f:
        vulnerable_classes = pickle.load(f)
    logger.info("Vulnerable classes are {}".format(vulnerable_classes))
    vulnerable_classes_random = vulnerable_classes[P_VALUE_COLUMN]
    mpl.rcParams.update({'font.size': 13, "font.family": "serif",
                         'font.serif': ['Times New Roman'] + plt.rcParams['font.serif']})

    logger.info("Plotting Results are {}".format(result_dirs.model_result_file_path))
    data_frame = pd.read_csv(result_dirs.model_result_file_path, index_col=0)
    logger.info("Accuracies are {}".format(data_frame.head()))
    params = dict(loc='upper center', bbox_to_anchor=(0.6, -0.1), ncol=4, fancybox=False, shadow=True,
                  facecolor='white', edgecolor='k', fontsize=10)

    extension = 'png'
    plot_learning_curves_importances(result_dirs, csv_reader, vulnerable_classes_random, extension=extension)

    plts = bar_grid_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iterations_dict[N_SPLITS]), result_dirs.plots_folder,
                                figsize=sfigsize, extension=extension)
    plts = classwise_barplot_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iterations_dict[N_SPLITS]),
                                         result_dirs.plots_folder, figsize=sfigsize, extension=extension)

    logger.info("Finished Plotting")
