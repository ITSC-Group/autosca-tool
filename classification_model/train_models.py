import argparse
import logging
import os
import pickle
import warnings
from datetime import datetime
from itertools import product

import numpy as np
import pandas as pd
from scipy.stats import wilcoxon
from sklearn.ensemble import RandomForestClassifier
from sklearn.utils import check_random_state
from statsmodels.stats.multitest import multipletests

from pycsca.baseline import RandomClassifier
from pycsca.classification_test import optimize_search_cv
from pycsca.classifiers import classifiers_space
from pycsca.csv_reader import CSVReader
from pycsca.plot_utils import custom_dict
from pycsca.statistical_tests import corrected_dependent_ttest
from pycsca.utils import setup_logging, print_dictionary

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
    parser.add_argument('-i', '--iterations', type=int, default=10,
                        help='Number of iteration for parameter optimization')
    parser.add_argument('-se', '--skipexisting', type=bool, default=False,
                        help='Number of iteration for parameter optimization')
    args = parser.parse_args()
    cv_iter = int(args.cv_iterations)
    n_iter = int(args.iterations)
    skip_existing = int(args.skipexisting)
    folder = args.folder

    log_file = os.path.join(folder, 'learning.log')
    setup_logging(log_path=log_file)
    logger = logging.getLogger("LearningExperiment")

    csv_reader = CSVReader(folder=args.folder, seed=42)
    dataset = args.folder.split('/')[-1]

    columns = ['Dataset', 'Model', 'Accuracy', 'Accuracy-std', 'F1-Score', 'F1-Score-std', "ttest-pval",
               'wilcoxon-pval']
    p_vals_corrected = []
    rejected = []
    p_vals_corrected2 = []
    rejected2 = []
    vulnerable_classes = []
    report = ''
    random_state = check_random_state(42)
    models_folder = os.path.join(folder, 'Models')
    if not os.path.exists(models_folder):
        os.mkdir(models_folder)
    file_path = os.path.join(folder, 'Model Results.csv')
    final = []
    if os.path.exists(file_path):
        df = pd.read_csv(file_path, index_col=0)
        logger.info("The results already exists")
    else:
        df = None
    cls_file = os.path.join(folder, 'Model Accuracies.pickle')
    if os.path.exists(cls_file):
        with open(cls_file, 'rb') as f:
            accs_dict = pickle.load(f)
        f.close()
    else:
        accs_dict = dict()
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        start = datetime.now()
        dt_string = start.strftime("%d/%m/%Y %H:%M:%S")
        logger.info("#############################################################################")
        logger.info("Starting time = {}".format(dt_string))
        i = 0
        x, y = csv_reader.get_data_class(class_label=j, missing_ccs_fin=missing_ccs_fin)
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping Multi-Class")
            continue
        # estimators = []
        raccs = None
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        for classifier, params, search_space in classifiers_space:
            logger.info("#############################################################################")
            logger.info("Classifier {}, running for class {}".format(classifier.__name__, label))
            if df is not None and skip_existing:
                mdf = df[(df['Dataset'] == label) & (df['Model'] == classifier.__name__)]
                if mdf.shape[0] == 1:
                    logger.info("Classifier {}, is already evaluated for label {}".format(classifier.__name__, label))
                    d = dict(zip(columns, mdf.values.flatten()))
                    final.append(mdf[columns].values.flatten())
                    logger.info(mdf[columns].values.flatten())
                    logger.info("row {}".format(print_dictionary(d)))
                    if raccs is None:
                        raccs = accs_dict[classifier.__name__ + '-' + label]
                    continue

            params['random_state'] = random_state
            n_classes = csv_reader.n_labels
            test_size = 0.3
            if int(0.3 * y.shape[0]) < n_classes:
                test_size = (n_classes * 2) / y.shape[0]
            accs, f1s, best_estimator = optimize_search_cv(classifier, params, search_space, test_size, cv_iter, n_iter
                                                           , x, y)
            accs_dict[classifier.__name__ + '-' + label] = accs
            if classifier.__name__ == RandomClassifier.__name__:
                raccs = accs
            _, _, p_ttest = corrected_dependent_ttest(raccs, accs, 1 - test_size, test_size, 0.01)
            val1 = accs
            val2 = raccs
            if classifier.__name__ == RandomClassifier.__name__:
                w, p_wilcox = 0.0, 1.0
                p_ttest = 1.0
            else:
                w, p_wilcox = wilcoxon(val1, val2)
                logger.info("Classifier {}, p_vals {}".format(classifier.__name__, (p_ttest, p_wilcox)))
            vals = np.array([np.mean(accs), np.std(accs), np.mean(f1s), np.std(f1s)]).round(4)
            oneRow = [label, classifier.__name__] + list(vals) + [p_ttest, p_wilcox]
            final.append(oneRow)
            # if classifier.__name__ != RandomClassifier.__name__:
            # estimators.append((classifier.__name__, best_estimator))
            if classifier.__name__ == RandomForestClassifier.__name__:
                model = best_estimator
            logger.info("Classifier {}, class {}, accuracy {}".format(oneRow[1], label, np.mean(accs)))
            # print("Classifier {}, class {}, accuracy {}".format(oneRow[1], label, np.mean(accs)))
            name = classifier.__name__.lower() + '-' + '_'.join(label.lower().split(' ')) + '.pickle'
            file_name = os.path.join(models_folder, name)
            with open(file_name, 'wb') as f:
                pickle.dump(best_estimator, f)

        data_frame = pd.DataFrame(final, columns=columns)
        data_frame.loc[data_frame['Model'] == RandomClassifier.__name__, ['ttest-pval', 'wilcoxon-pval']] = 1.0
        p_vals = data_frame[data_frame['Dataset'] == label]['ttest-pval'].values
        reject, pvals_corrected, _, alpha = multipletests(p_vals, 0.05, method='holm', is_sorted=False)
        p_vals_corrected.extend(pvals_corrected)
        rejected.extend(reject)
        if np.any(reject):
            vulnerable_classes.append(label)
            class_n_format = 'The server is Vulnerable to Class {} \n'.format(label)
            logger.info(class_n_format)
            report = report + class_n_format

        p_vals = data_frame[data_frame['Dataset'] == label]['wilcoxon-pval'].values
        reject2, pvals_corrected, _, alpha = multipletests(p_vals, 0.05, method='holm', is_sorted=False)
        p_vals_corrected2.extend(pvals_corrected)
        rejected2.extend(reject2)
        end = datetime.now()
        total = (end - start).total_seconds() * 1000
        logger.info("Total Time taken by the class {} is {} milliseconds "
                    "and {} seconds".format(label, total, total / 1000))
        logger.info("#######################################################################")
        logger.info("{}: {}".format(len(rejected), data_frame.shape))

    data_frame['p-values-ttest_corrected'] = p_vals_corrected
    data_frame['Rejected'] = rejected

    data_frame['p-values-wilcoxin_corrected'] = p_vals_corrected2
    data_frame['Rejected-wilcoxin'] = rejected2

    data_frame['rank'] = data_frame['Model'].map(custom_dict)
    data_frame.sort_values(by=['Dataset', 'rank'], ascending=[True, True], inplace=True)
    del data_frame['rank']
    data_frame.to_csv(file_path)

    if report != '':
        report = report + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report = report + 'The Server is Not-Vulnerable to Side Channel attacks'

    with open(os.path.join(folder, 'Report.txt'), "w") as text_file:
        text_file.write(report)
    with open(os.path.join(folder, 'Model Accuracies.pickle'), 'wb') as file:
        pickle.dump(accs_dict, file)
    with open(os.path.join(folder, 'Vulnerable Classes.pickle'), "wb") as class_file:
        pickle.dump(vulnerable_classes, class_file)
