import argparse
import logging
import numpy as np
import os
import pandas as pd
import pickle
from itertools import product
from scipy.stats import fisher_exact
from statsmodels.stats.multitest import multipletests

from classification_model.result_directories import ResultDirectories
from pycsca import *
from utils import test_size, cols_metrics, cols_pvals, columns


def holm_bonferroni(data_frame, label, pval_col):
    searchFor = [RandomClassifier.__name__, MajorityVoting.__name__, PriorClassifier.__name__]
    df = data_frame[~data_frame['Model'].str.contains('|'.join(searchFor))]
    p_vals = df[df['Dataset'] == label][pval_col].values
    reject, pvals_corrected, _, alpha = multipletests(p_vals, 0.01, method='holm', is_sorted=False)
    reject = [False] * len(searchFor) + list(reject)
    pvals_corrected = [1.0] * len(searchFor) + list(pvals_corrected)
    return p_vals, pvals_corrected, reject


def update_report(report_string, rejected, p_values, label):
    append_string = 'The server is Vulnerable to Class {} \n'.format(label)
    report_string = report_string + append_string
    append_string = 'Highest p-value {}, Lowest p-value {}, Number of Algorithms {} \n'.format(np.max(p_values),
                                                                                               np.min(p_values),
                                                                                               np.sum(rejected))
    report_string = report_string + append_string
    report_string = report_string + "********************************************************************************\n"
    return report_string

if __name__== "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-f', '--folder', required=True,
                        help='Folder that contains the input files Packets.pcap and Client Requests.csv '
                             'and that the output files will be written to')
    parser.add_argument('-cvt', '--cv_technique', choices=cv_choices, default='auto',
                        help='Cross-Validation Technique to be used for generating evaluation samples')

    args = parser.parse_args()
    folder = args.folder
    cv_technique = str(args.cv_technique)
    result_dirs = ResultDirectories(folder=folder)
    setup_logging(log_path=result_dirs.pvalue_cal_log_file)
    logger = logging.getLogger("P-Value Calculation")
    logger.info("Arguments {}".format(args))

    csv_reader = CSVReader(folder=folder, seed=42)
    csv_reader.plot_class_distribution()
    dataset = args.folder.split('/')[-1]
    vulnerable_classes = dict()
    report_string = ''
    for k in cols_pvals:
        vulnerable_classes[k] = []

    logger.info("Starting the p-value calculation")
    if os.path.exists(result_dirs.accuracies_file):
        with open(result_dirs.accuracies_file, 'rb') as f:
            metrics_dictionary = pickle.load(f)
        f.close()
    else:
        raise ValueError("The learning simulations are not done yet")
    cv_iterations_dict = metrics_dictionary[CV_ITERATIONS_LABEL]
    final = []
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping p-val calculation Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        try:
            KEY = SCORE_KEY_FORMAT.format(RandomClassifier.__name__, label)
            random_accs = metrics_dictionary[KEY][ACCURACY]
            KEY = SCORE_KEY_FORMAT.format(MajorityVoting.__name__, label)
            majority_accs = metrics_dictionary[KEY][ACCURACY]
            KEY = SCORE_KEY_FORMAT.format(PriorClassifier.__name__, label)
            prior_accs = metrics_dictionary[KEY][ACCURACY]
        except:
            logger.info("Skipping p-val calculation class label {}".format(label))
            continue
        for classifier, params, search_space in classifiers_space:
            cls_name = classifier.__name__
            KEY = SCORE_KEY_FORMAT.format(cls_name, label)
            scores = metrics_dictionary[KEY]
            logger.info("#############################################################################")
            logger.info("Classifier {}, p-value calculation {}".format(cls_name, label))
            accuracies = scores[ACCURACY]
            confusion_matrices = scores[CONFUSION_MATRICES]
            cm_single = scores[CONFUSION_MATRIX_SINGLE]
            if cv_technique == 'kccv':
                n_training_folds = cv_iterations_dict[label] - 1
                n_test_folds = 1
            elif cv_technique == 'mccv':
                n_training_folds = 1 - test_size
                n_test_folds = test_size
            else:
                raise ValueError('Cross-Validation technique is does not exist should be {} or {}'.format(cv_choices[0],
                                                                                                          cv_choices[1]))
            if np.any(np.isnan(accuracies)):
                p_random_cttest, p_majority_cttest, p_prior_cttest, p_random_ttest, p_majority_ttest, p_prior_ttest, \
                p_random_wilcox, p_majority_wilcox, p_prior_wilcox = 1, 1, 1, 1, 1, 1, 1, 1, 1
            else:
                p_random_cttest = paired_ttest(random_accs, accuracies, n_training_folds, n_test_folds, correction=True)
                p_majority_cttest = paired_ttest(majority_accs, accuracies, n_training_folds, n_test_folds,
                                                 correction=True)
                p_prior_cttest = paired_ttest(prior_accs, accuracies, n_training_folds, n_test_folds, correction=True)

                p_random_ttest = paired_ttest(random_accs, accuracies, n_training_folds, n_test_folds, correction=False)
                p_majority_ttest = paired_ttest(majority_accs, accuracies, n_training_folds, n_test_folds,
                                                correction=False)
                p_prior_ttest = paired_ttest(prior_accs, accuracies, n_training_folds, n_test_folds, correction=False)

                p_majority_wilcox = wilcoxon_signed_rank_test(majority_accs, accuracies)
                p_random_wilcox = wilcoxon_signed_rank_test(random_accs, accuracies)
                p_prior_wilcox = wilcoxon_signed_rank_test(prior_accs, accuracies)

            _, pvalue_single = fisher_exact(cm_single)
            confusion_matrix_sum = confusion_matrices.sum(axis=0)
            _, pvalue_sum = fisher_exact(confusion_matrix_sum)
            p_values = np.array([fisher_exact(cm)[1] for cm in confusion_matrices])
            pvalue_mean = np.mean(p_values)
            pvalue_median = np.median(p_values)
            logger.info("P-values {}".format(p_values))

            reject, pvals_corrected, _, alpha = multipletests(p_values, 0.01, method='holm', is_sorted=False)
            logger.info("Holm Bonnferroni Rejected Hypothesis: {} min: {} max: {}".format(np.sum(reject),
                                                                                          np.min(pvals_corrected),
                                                                                          np.max(pvals_corrected)))

            logger.info(" Corrected p_values {}".format(pvals_corrected))
            final_pvals = [pvalue_single, pvalue_sum, pvalue_median, pvalue_mean, np.median(pvals_corrected),
                           p_random_cttest, p_majority_cttest, p_prior_cttest, p_random_ttest, p_majority_ttest,
                           p_prior_ttest, p_random_wilcox, p_majority_wilcox, p_prior_wilcox]
            vals = []
            for k in METRICS:
                v = scores[k]
                vals.extend([np.mean(v).round(4), np.std(v).round(4)])
            d = dict(zip(cols_metrics + cols_pvals, vals + final_pvals))
            logger.info("Classifier {}, Metrics {}".format(cls_name, print_dictionary(d)))
            one_row = [label, cls_name] + vals + final_pvals
            final.append(one_row)

    data_frame = pd.DataFrame(final, columns=columns)
    data_frame['rank'] = data_frame[MODEL].map(custom_dict)
    data_frame.sort_values(by=[DATASET, 'rank'], ascending=[True, True], inplace=True)
    del data_frame['rank']
    data_frame.to_csv(result_dirs.model_result_file_path)
    data_frame = pd.DataFrame(final, columns=columns)
    for pval_col in cols_pvals:
        data_frame[pval_col + '-rejected'] = False

    final = []
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping holm bonferroni correction calculation Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'
        if label not in data_frame['Dataset'].unique():
            logger.info("Skipping the p-val calculation class label {}".format(label))
            continue
        one_row = [label]
        for pval_col in cols_pvals:
            p_vals, pvals_corrected, reject = holm_bonferroni(data_frame, label, pval_col=pval_col)
            data_frame.loc[data_frame['Dataset'] == label, [pval_col + '-rejected']] = reject
            # print(label, pval_col, reject)
            # print(data_frame[data_frame['Dataset'] == label][[pval_col + '-corrected', pval_col + '-rejected']])
            # print('##############################################################################')
            one_row.extend([np.any(reject), np.sum(reject)])
            if np.any(reject):
                vulnerable_classes[pval_col].append(label)
                logger.info("Adding class {} for pval {}".format(label, pval_col))
                if pval_col == CTTEST_PVAL + '-random':
                    report_string = update_report(report_string, reject, pvals_corrected, label)
        final.append(one_row)

    logger.info(print_dictionary(vulnerable_classes))
    data_frame['rank'] = data_frame['Model'].map(custom_dict)
    data_frame.sort_values(by=['Dataset', 'rank'], ascending=[True, True], inplace=True)
    del data_frame['rank']
    data_frame.to_csv(result_dirs.model_result_file_path)

    columns = [DATASET] + list(np.array([[c, c + '-count'] for c in cols_pvals]).flatten())
    data_frame = pd.DataFrame(final, columns=columns)
    data_frame.sort_values(by=[DATASET], ascending=[True], inplace=True)
    data_frame.to_csv(result_dirs.result_file_path)

    with open(result_dirs.vulnerable_file, "wb") as class_file:
        pickle.dump(vulnerable_classes, class_file)

    if report_string != '':
        report_string = report_string + 'The Server is Vulnerable to Side Channel attacks'
    else:
        report_string = report_string + 'The Server is Not-Vulnerable to Side Channel attacks'

    text_file = open(result_dirs.report_file, "w")
    n = text_file.write(report_string)
    text_file.close()
