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

from pycsca.csv_reader import CSVReader
from pycsca.plot_utils import classwise_barplot_for_dataset, \
    bar_grid_for_dataset, plot_learning_curves_importances
from pycsca.utils import setup_logging

ACCURACY = 'Accuracy'

MULTI_CLASS = 'Multi-Class'

if __name__ == "__main__":
    warnings.simplefilter("ignore")
    warnings.simplefilter('always', category=UserWarning)
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--folder', required=True,
                        help='Folder that contains the input files Packets.pcap and Client Requests.csv '
                             'and that the output files will be written to')
    parser.add_argument('-cv', '--cv_iterations', type=int, default=30,
                        help='Number of iteration for training and testing the models')
    choices = ['kccv', 'mccv']
    parser.add_argument('-cvt', '--cv_technique', choices=choices, default='mccv',
                        help='Cross-Validation Technique to be used for generating evaluation samples')
    args = parser.parse_args()
    cv_iter = int(args.cv_iterations)
    folder = args.folder
    csv_reader = CSVReader(folder=folder, seed=42)
    dataset = args.folder.split('/')[-1]
    random_state = check_random_state(42)
    figsize = (7, 5)
    sfigsize = (4, 4)
    plots_folder = os.path.join(folder, 'Plots')
    if not os.path.exists(plots_folder):
        os.mkdir(plots_folder)
    models_folder = os.path.join(folder, 'Models')
    imp_folder = os.path.join(plots_folder, 'Feature Importance')
    if not os.path.exists(imp_folder):
        os.mkdir(imp_folder)
    log_file = os.path.join(folder, 'plotting.log')
    setup_logging(log_path=log_file)
    logger = logging.getLogger("Plotting")
    with open(os.path.join(folder, 'Vulnerable Classes.pickle'), 'rb') as f:
        vulnerable_classes = pickle.load(f)
    logger.info("Vulnerable classes are {}".format(vulnerable_classes))
    mpl.rcParams.update({'font.size': 13, "font.family": "serif",
                         'font.serif': ['Times New Roman'] + plt.rcParams['font.serif']})

    file_path = os.path.join(folder, 'Model Results.csv')
    logger.info("Plotting Results are {}".format(file_path))
    data_frame = pd.read_csv(file_path, index_col=0)
    logger.info("Accuracies are {}".format(data_frame.head()))
    params = dict(loc='upper center', bbox_to_anchor=(0.6, -0.1), ncol=4, fancybox=False, shadow=True,
                  facecolor='white', edgecolor='k', fontsize=10)

    extension = 'png'
    plts = bar_grid_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iter), plots_folder, figsize=sfigsize,
                                extension=extension)
    plts = classwise_barplot_for_dataset(data_frame, ACCURACY, np.sqrt(cv_iter), plots_folder, figsize=sfigsize,
                                         extension=extension)

    lrcurve_folder = os.path.join(plots_folder, 'Learning Curves')
    if not os.path.exists(lrcurve_folder):
        os.mkdir(lrcurve_folder)
    imp_folder = os.path.join(plots_folder, 'Feature Importance')
    if not os.path.exists(imp_folder):
        os.mkdir(imp_folder)
    plot_learning_curves_importances(models_folder, csv_reader, vulnerable_classes, lrcurve_folder, imp_folder,
                                     extension=extension)
    logger.info("Finished Plotting")
