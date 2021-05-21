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
    choices = ['kccv', 'mccv']
    parser.add_argument('-cvt', '--cv_technique', choices=choices, default='mccv',
                        help='Cross-Validation Technique to be used for generating evaluation samples')
    parser.add_argument('-cv', '--cv_iterations', type=int, default=30,
                        help='Number of iteration for training and testing the models')
    args = parser.parse_args()
    cv_iter = int(args.cv_iterations)
    folder = args.folder
    cv_technique = str(args.cv_technique)
    csv_reader = CSVReader(folder=folder, seed=42)
    dataset = args.folder.split('/')[-1]
    random_state = check_random_state(42)

    figsize = (7, 5)
    sfigsize = (4, 4)
    subfolder = cv_technique.upper()
    models_folder = os.path.join(folder, subfolder, 'Models')

    plots_folder = os.path.join(folder, subfolder, 'Plots')
    create_dir_recursively(plots_folder)
    imp_folder = os.path.join(plots_folder, 'Feature Importance')
    create_dir_recursively(imp_folder)

    log_file = os.path.join(folder, subfolder, 'plotting.log')
    setup_logging(log_path=log_file, level=logging.ERROR)
    logger = logging.getLogger("Plotting")
    logger.info("Arguments {}".format(args))

    with open(os.path.join(folder, subfolder, 'Vulnerable Classes.pickle'), 'rb') as f:
        vulnerable_classes = pickle.load(f)
    logger.info("Vulnerable classes are {}".format(vulnerable_classes))
    mpl.rcParams.update({'font.size': 13, "font.family": "serif",
                         'font.serif': ['Times New Roman'] + plt.rcParams['font.serif']})

    file_path = os.path.join(folder, subfolder, 'Model Results.csv')
    logger.info("Plotting Results are {}".format(file_path))
    data_frame = pd.read_csv(file_path, index_col=0)
    logger.info("Accuracies are {}".format(data_frame.head()))
    params = dict(loc='upper center', bbox_to_anchor=(0.6, -0.1), ncol=4, fancybox=False, shadow=True,
                  facecolor='white', edgecolor='k', fontsize=10)

    extension = 'png'
    plts = bar_grid_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iter), plots_folder, figsize=sfigsize,
                                extension=extension, logger=logger)
    plts = classwise_barplot_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iter), plots_folder, figsize=sfigsize,
                                         extension=extension, logger=logger)

    lrcurve_folder = os.path.join(plots_folder, 'Learning Curves')
    if not os.path.exists(lrcurve_folder):
        os.mkdir(lrcurve_folder)
    imp_folder = os.path.join(plots_folder, 'Feature Importance')
    if not os.path.exists(imp_folder):
        os.mkdir(imp_folder)
    plot_learning_curves_importances(models_folder, csv_reader, vulnerable_classes, lrcurve_folder, imp_folder,
                                     extension=extension, logger=logger)
    logger.info("Finished Plotting")
