import argparse
import logging
import os
import pickle
import warnings
from datetime import datetime
from itertools import product

import numpy as np
import pandas as pd
from scipy.stats import wilcoxon, fisher_exact
from sklearn.dummy import DummyClassifier
from sklearn.model_selection import StratifiedKFold, StratifiedShuffleSplit
from sklearn.utils import check_random_state
from statsmodels.stats.multitest import multipletests

from pycsca.baseline import MajorityVoting, RandomClassifier, PriorClassifier
from pycsca.classification_test import optimize_search_cv
from pycsca.classifiers import classifiers_space
from pycsca.constants import *
from pycsca.csv_reader import CSVReader
from pycsca.plot_utils import custom_dict
from pycsca.statistical_tests import corrected_dependent_ttest
from pycsca.utils import setup_logging, print_dictionary, create_dir_recursively


def holm_bonferroni(data_frame, label, pval_col):
    searchFor = [RandomClassifier.__name__, MajorityVoting.__name__, PriorClassifier.__name__]
    df = data_frame[~data_frame['Model'].str.contains('|'.join(searchFor))]
    p_vals = df[df['Dataset'] == label][pval_col].values
    reject, pvals_corrected, _, alpha = multipletests(p_vals, 0.05, method='holm', is_sorted=False)
    reject = [False] * len(searchFor) + list(reject)
    pvals_corrected = [1.0] * len(searchFor) + list(pvals_corrected)
    return p_vals, pvals_corrected, reject


def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


def update_report(report_string, v_classes, rejected, p_values, label):
    v_classes.append(label)
    append_string = 'The server is Vulnerable to Class {} \n'.format(label)
    report_string = report_string + append_string
    append_string = 'Highest p-value {}, Lowest p-value {}, Number of Algorithms {} \n'.format(np.max(p_values),
                                                                                               np.min(p_values),
                                                                                               np.sum(rejected))
    report_string = report_string + append_string
    report_string = report_string + "********************************************************************************\n"
    return report_string, v_classes

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
                        help='Number of iteration for Hyper-parameter optimization')
    choices = ['kccv', 'mccv']
    parser.add_argument('-cvt', '--cv_technique', choices=choices, default='mccv',
                        help='Cross-Validation Technique to be used for generating evaluation samples')
    parser.add_argument('-se', '--skipexisting', type=str2bool, nargs='?',
                        const=True, default=False,
                        help='Number of iteration for parameter optimization')
    args = parser.parse_args()
    cv_iter = int(args.cv_iterations)
    hp_iterations = int(args.iterations)
    skip_existing = args.skipexisting
    folder = args.folder
    cv_technique = str(args.cv_technique)
    test_size = 0.3
    random_state = check_random_state(42)
    subfolder = cv_technique.upper()
    log_file = os.path.join(folder, subfolder, 'learning.log')
    create_dir_recursively(log_file, is_file_path=True)
    setup_logging(log_path=log_file)
    logger = logging.getLogger("LearningExperiment")
    logger.info("Arguments {}".format(args))
    csv_reader = CSVReader(folder=args.folder, seed=42)
    csv_reader.plot_class_distribution()
    dataset = args.folder.split('/')[-1]
    t_pval = "ttest-pval"
    w_pval = 'wilcoxon-pval'
    fisher = 'fisher-pval'
    cols_metrics = ['Dataset', 'Model', ACCURACY, '{}-std'.format(ACCURACY), F1SCORE, "{}-std".format(F1SCORE),
                    AUC_SCORE, "{}-std".format(AUC_SCORE), COHENKAPPA, "{}-std".format(COHENKAPPA), MCC,
                    "{}-std".format(MCC), INFORMEDNESS, "{}-std".format(INFORMEDNESS)]
    cols_pvals = [fisher + '-single', fisher + '-median', fisher + '-mean', fisher + '-sum', t_pval + '-random',
                  t_pval + '-majority', t_pval + '-prior', w_pval + '-random', w_pval + '-majority', w_pval + '-prior']
    vulnerable_classes = []
    vulnerable_classes_majority = []
    vulnerable_classes_prior = []
    vulnerable_classes_fisher = []
    vulnerable_classes_fisher_single = []
    report_random = ''
    report_majority = ''
    report_prior = ''
    report_fisher = ''
    report_fisher_single = ''
    missed_classes = '################################################################################################'

    models_folder = os.path.join(folder, subfolder, 'Models')
    create_dir_recursively(models_folder)

    df_file_path = os.path.join(folder, subfolder, 'Model Results.csv')
    final = []
    if os.path.exists(df_file_path):
        df = pd.read_csv(df_file_path, index_col=0)
        logger.info("The results already exists")
        # searchFor = [RandomClassifier.__name__, MajorityVoting.__name__, PriorClassifier.__name__]
        # df = df[~df['Model'].str.contains('|'.join(searchFor))]
    else:
        df = None
    accuracies_file = os.path.join(folder, subfolder, 'Model Accuracies.pickle')
    if os.path.exists(accuracies_file):
        with open(accuracies_file, 'rb') as f:
            metrics_dictionary = pickle.load(f)
        f.close()
    else:
        metrics_dictionary = dict()

    report_file_random = os.path.join(folder, subfolder, 'Report Random.txt')
    vulnerable_file = os.path.join(folder, subfolder, 'Vulnerable Classes.pickle')

    report_file_majority = os.path.join(folder, subfolder, 'Report Majority.txt')
    vulnerable_majority_file = os.path.join(folder, subfolder, 'Vulnerable Classes Majority.pickle')

    report_file_prior = os.path.join(folder, subfolder, 'Report Prior.txt')
    vulnerable_prior_file = os.path.join(folder, subfolder, 'Vulnerable Classes Prior.pickle')

    report_file_fisher = os.path.join(folder, subfolder, 'Report Fisher.txt')
    vulnerable_fisher_file = os.path.join(folder, subfolder, 'Vulnerable Classes Fisher.pickle')

    report_file_fisher_single = os.path.join(folder, subfolder, 'Report Fisher Single.txt')
    vulnerable_fisher_single_file = os.path.join(folder, subfolder, 'Vulnerable Classes Fisher Single.pickle')

    start = datetime.now()
    if cv_technique == 'kccv':
        cv_iterator = StratifiedKFold(n_splits=cv_iter, shuffle=True, random_state=random_state)
    elif cv_technique == 'mccv':
        cv_iterator = StratifiedShuffleSplit(n_splits=cv_iter, test_size=test_size, random_state=random_state)
    else:
        raise ValueError(
            'Cross-Validation technique is does not exist should be {} or {}'.format(choices[0], choices[1]))
    logger.info('cv_iterator {}'.format(cv_iterator))
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        start_label = datetime.now()
        dt_string = start_label.strftime("%d/%m/%Y %H:%M:%S")
        logger.info("#############################################################################")
        logger.info("Starting time = {}".format(dt_string))
        i = 0
        x, y = csv_reader.get_data_class(class_label=j, missing_ccs_fin=missing_ccs_fin)
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        if y.shape[0] < 3 or len(np.unique(y)) == 1:
            logger.info("Skipping the class label {}".format(label))
            if len(np.unique(y)) == 1:
                logger.info("Only one class instances")
            if y.shape[0] < 3:
                logger.info("Very less number of instances {}".format(y.shape[0]))
            continue
        if y.shape[0] < 2 * cv_iter and cv_technique == 'kccv':
            ones = np.count_nonzero(y)
            cv_iter = np.min([ones, y.shape[0] - ones])
            logger.info("For label {} new cv {} from {}".format(label, cv_iter, [ones, y.shape[0] - ones]))
            cv_iterator = StratifiedKFold(n_splits=cv_iter, shuffle=True, random_state=random_state)

        for classifier, params, search_space in classifiers_space:
            logger.info("#############################################################################")
            logger.info("Classifier {}, running for class {}".format(classifier.__name__, label))
            if skip_existing and df is not None:
                mdf = df[(df['Dataset'] == label) & (df['Model'] == classifier.__name__)][cols_metrics]
            if skip_existing and (df is not None and mdf.shape[0] == 1):
                logger.info("Classifier {}, is already evaluated for label {}".format(classifier.__name__, label))
                d = dict(zip(cols_metrics, mdf.values.flatten()))
                # final.append(mdf[cols].values.flatten())
                oneRow = list(mdf[cols_metrics].values.flatten())
                logger.info("row {}".format(print_dictionary(d)))
            else:
                params['random_state'] = random_state
                n_classes = csv_reader.n_labels
                if int(test_size * y.shape[0]) < n_classes:
                    test_size = (n_classes * 2) / y.shape[0]
                scores, best_estimator = optimize_search_cv(classifier, params, search_space, cv_iterator, hp_iterations
                                                            , x, y, random_state=random_state)
                metrics_dictionary['{}-scores-{}'.format(classifier.__name__, label)] = scores
                vals = []
                for k in cols_metrics[2:][::2]:
                    if 'std' not in k:
                        v = scores[k]
                        vals.extend([np.mean(v).round(4), np.std(v).round(4)])
                oneRow = [label, classifier.__name__] + list(vals)
                d = dict(zip(cols_metrics[2:], vals))
                logger.info("Classifier {}, label {}, Evaluations {}".format(classifier.__name__, label,
                                                                             print_dictionary(d)))
                # print("Classifier {}, class {}, accuracy {}".format(oneRow[1], label, np.mean(accs)))
                name = classifier.__name__.lower() + '-' + '_'.join(label.lower().split(' ')) + '.pickle'
                file_name = os.path.join(models_folder, name)
                with open(file_name, 'wb') as f:
                    pickle.dump(best_estimator, f)
            final.append(oneRow)
        end_label = datetime.now()
        total = (end_label - start_label).total_seconds()
        logger.info(
            "Time taken for evaluation of labels {} is {} seconds and {} hours".format(label, total, total / 3600))
        logger.info("#######################################################################")
    end = datetime.now()
    total = (end - start).total_seconds()
    logger.info("Time taken for finishing the learning task is {} seconds and {} hours".format(total, total / 3600))
    logger.info("#######################################################################")

    data_frame = pd.DataFrame(final, columns=cols_metrics)

    for pval_col in cols_pvals:
        data_frame[pval_col] = 1.0
        data_frame[pval_col + '-corrected'] = 1.0
        data_frame[pval_col + '-rejected'] = False

    data_frame.to_csv(df_file_path)
    logger.info("Starting the p-value calculation")
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping p-val calculation Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        if label not in data_frame['Dataset'].unique():
            logger.info("Skipping p-val calculation class label {}".format(label))
            continue
        random_accs = metrics_dictionary['{}-scores-{}'.format(RandomClassifier.__name__, label)][ACCURACY]
        majority_accs = metrics_dictionary['{}-scores-{}'.format(MajorityVoting.__name__, label)][ACCURACY]
        prior_accs = metrics_dictionary['{}-scores-{}'.format(PriorClassifier.__name__, label)][ACCURACY]
        for classifier, params, search_space in classifiers_space:
            logger.info("#############################################################################")
            logger.info("Classifier {}, p-value calculation {}".format(classifier.__name__, label))
            accuracies = metrics_dictionary['{}-scores-{}'.format(classifier.__name__, label)][ACCURACY]
            confusion_matrices = metrics_dictionary['{}-scores-{}'.format(classifier.__name__, label)][
                CONFUSION_MATRICES]
            cm_single = metrics_dictionary['{}-scores-{}'.format(classifier.__name__, label)][CONFUSION_MATRIX_SINGLE]
            if issubclass(classifier, DummyClassifier):
                p_random_ttest, p_majority_ttest, p_prior_ttest, p_random_wilcox, p_majority_wilcox, p_prior_wilcox = 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
            else:
                if cv_technique == 'kccv':
                    n_training_folds = cv_iter - 1
                    n_test_folds = 1
                elif cv_technique == 'mccv':
                    n_training_folds = 1 - test_size
                    n_test_folds = test_size
                else:
                    raise ValueError(
                        'Cross-Validation technique is does not exist should be {} or {}'.format(choices[0],
                                                                                                 choices[1]))
                p_random_ttest = corrected_dependent_ttest(random_accs, accuracies, n_training_folds, n_test_folds,
                                                           0.01)
                p_majority_ttest = corrected_dependent_ttest(majority_accs, accuracies, n_training_folds, n_test_folds,
                                                             0.01)
                p_prior_ttest = corrected_dependent_ttest(prior_accs, accuracies, n_training_folds, n_test_folds, 0.01)
                try:
                    _, p_majority_wilcox = wilcoxon(majority_accs, accuracies, correction=True)
                except Exception as e:
                    logger.info('Accuracies are exactly same {}'.format(str(e)))
                    p_majority_wilcox = 1.0

                try:
                    _, p_random_wilcox = wilcoxon(random_accs, accuracies, correction=True)
                except Exception as e:
                    logger.info('Accuracies are exactly same {}'.format(str(e)))
                    p_random_wilcox = 1.0

                try:
                    _, p_prior_wilcox = wilcoxon(prior_accs, accuracies, correction=True)
                except Exception as e:
                    logger.info('Accuracies are exactly same {}'.format(str(e)))
                    p_prior_wilcox = 1.0

            _, pvalue_sum = fisher_exact(confusion_matrices.sum(axis=0))
            _, pvalue_single = fisher_exact(cm_single)
            p_values = np.array([fisher_exact(cm)[1] for cm in confusion_matrices])
            pvalue_mean = np.mean(p_values)
            pvalue_median = np.median(p_values)
            # cols_pvals = [fisher + '-single', fisher + '-median', fisher + '-mean', fisher + '-sum', t_pval + '-random',
            #             t_pval + '-majority', t_pval + '-prior', w_pval + '-random', w_pval + '-majority',
            #             w_pval + '-prior']
            final_pvals = [pvalue_single, pvalue_median, pvalue_mean, pvalue_sum, p_random_ttest, p_majority_ttest,
                           p_prior_ttest, p_random_wilcox, p_majority_wilcox, p_prior_wilcox]
            d = dict(zip(cols_pvals, final_pvals))
            logger.info("Classifier {}, p_vals {}".format(classifier.__name__, print_dictionary(d)))
            vals = data_frame.loc[
                (data_frame['Dataset'] == label) & (data_frame['Model'] == classifier.__name__), cols_metrics].values
            vals = list(np.array(vals).flatten())
            d = dict(zip(cols_metrics, vals))
            logger.info("Classifier {}, metrics {}".format(classifier.__name__, print_dictionary(d)))
            data_frame.loc[(data_frame['Dataset'] == label) & (
                    data_frame['Model'] == classifier.__name__), cols_pvals] = final_pvals

    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping holm bonferroni correction calculation Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        if label not in data_frame['Dataset'].unique():
            logger.info("Skipping the p-val calculation class label {}".format(label))
            missed_classes = missed_classes + "\nSkipping the class label {} The reason could be that there were no " \
                                              "handshakes for this class".format(label)
            continue
        for pval_col in cols_pvals:
            p_vals, pvals_corrected, reject = holm_bonferroni(data_frame, label, pval_col=pval_col)
            data_frame.loc[data_frame['Dataset'] == label, [pval_col + '-corrected']] = pvals_corrected
            data_frame.loc[data_frame['Dataset'] == label, [pval_col + '-rejected']] = reject
            # print(label, pval_col, reject)
            # print(data_frame[data_frame['Dataset'] == label][[pval_col + '-corrected', pval_col + '-rejected']])
            # print('##############################################################################')
            if np.any(reject) and pval_col == t_pval + '-random':
                report_random, vulnerable_classes = update_report(report_random, vulnerable_classes, reject, p_vals,
                                                                  label)
            if np.any(reject) and pval_col == t_pval + '-majority':
                report_majority, vulnerable_classes_majority = update_report(report_majority,
                                                                             vulnerable_classes_majority,
                                                                             reject, p_vals, label)
            if np.any(reject) and pval_col == t_pval + '-prior':
                report_prior, vulnerable_classes_prior = update_report(report_prior, vulnerable_classes_prior,
                                                                       reject, p_vals, label)
            if cv_technique == 'mccv':
                c = fisher + '-mean'
            elif cv_technique == 'kccv':
                c = fisher + '-sum'
            if np.any(reject) and pval_col == c:
                report_fisher, vulnerable_classes_fisher = update_report(report_fisher, vulnerable_classes_fisher,
                                                                         reject, p_vals, label)
            if np.any(reject) and pval_col == fisher + '-single':
                report_fisher_single, vulnerable_classes_fisher_single = update_report(report_fisher_single,
                                                                                       vulnerable_classes_fisher_single,
                                                                                       reject, p_vals, label)

    data_frame['rank'] = data_frame['Model'].map(custom_dict)
    data_frame.sort_values(by=['Dataset', 'rank'], ascending=[True, True], inplace=True)
    del data_frame['rank']
    data_frame.to_csv(df_file_path)
    if report_random != '':
        report_random = report_random + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_random = report_random + 'The Server is Not-Vulnerable to Side Channel attacks'
    if report_majority != '':
        report_majority = report_majority + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_majority = report_majority + 'The Server is Not-Vulnerable to Side Channel attacks'
    if report_prior != '':
        report_prior = report_prior + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_prior = report_prior + 'The Server is Not-Vulnerable to Side Channel attacks'
    if report_fisher != '':
        report_fisher = report_fisher + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_fisher = report_fisher + 'The Server is Not-Vulnerable to Side Channel attacks'
    if report_fisher_single != '':
        report_fisher_single = report_fisher_single + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_fisher_single = report_fisher_single + 'The Server is Not-Vulnerable to Side Channel attacks'
    report_random = report_random + '\n' + missed_classes
    report_majority = report_majority + '\n' + missed_classes
    report_prior = report_prior + '\n' + missed_classes
    report_fisher = report_fisher + '\n' + missed_classes
    report_fisher_single = report_fisher_single + '\n' + missed_classes

    with open(report_file_random, "w") as text_file:
        text_file.write(report_random)
    with open(report_file_majority, "w") as text_file:
        text_file.write(report_majority)
    with open(report_file_prior, "w") as text_file:
        text_file.write(report_prior)
    with open(report_file_fisher, "w") as text_file:
        text_file.write(report_fisher)
    with open(report_file_fisher_single, "w") as text_file:
        text_file.write(report_fisher_single)

    with open(accuracies_file, 'wb') as file:
        pickle.dump(metrics_dictionary, file)

    with open(vulnerable_file, "wb") as class_file:
        pickle.dump(vulnerable_classes, class_file)
    with open(vulnerable_majority_file, "wb") as class_file:
        pickle.dump(vulnerable_classes_majority, class_file)
    with open(vulnerable_prior_file, "wb") as class_file:
        pickle.dump(vulnerable_classes_prior, class_file)
    with open(vulnerable_fisher_file, "wb") as class_file:
        pickle.dump(vulnerable_classes_fisher, class_file)
    with open(vulnerable_fisher_single_file, "wb") as class_file:
        pickle.dump(vulnerable_classes_fisher_single, class_file)