import argparse
import logging
import numpy as np
import os
import pickle
import warnings
from datetime import datetime
from itertools import product
from sklearn.model_selection import StratifiedKFold, StratifiedShuffleSplit
from sklearn.utils import check_random_state

from result_directories import ResultDirectories
from pycsca.classification_test import optimize_search_cv
from pycsca.classifiers import classifiers_space
from pycsca.constants import *
from pycsca.csv_reader import CSVReader
from pycsca.utils import setup_logging, print_dictionary


def print_accuracies(cls_name, label, scores):
    vals = []
    for k in METRICS:
        v = scores[k]
        vals.extend([np.mean(v).round(4), np.std(v).round(4)])
    d = dict(zip(cols_metrics, vals))
    logger.info("Classifier {}, label {}, Evaluations {}".format(cls_name, label, print_dictionary(d)))



def str2bool(v):
    if isinstance(v, bool):
        return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')


# Add a parameter of number jobs in start.sh with number of jobs
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
    parser.add_argument('-cvt', '--cv_technique', choices=cv_choices, default='auto',
                        help='Cross-Validation Technique to be used for generating evaluation samples')
    parser.add_argument('-nj', '--n_jobs', type=int, default=8, help='Number of jobs to be used for parallelism')
    parser.add_argument('-se', '--skipexisting', type=str2bool, nargs='?', const=True, default=False,
                        help='The decision to skip the learning task for the current configuration')
    parser.add_argument('-dl', '--debuglevel', choices=list(debug_levels.keys()), default=1,
                        help='The Debug level specifying if the Debug and Intermediate Result folder to be stored')
    args = parser.parse_args()
    cv_iterations = int(args.cv_iterations)
    hp_iterations = int(args.iterations)
    n_jobs = int(args.n_jobs)
    skip_existing = args.skipexisting
    folder = args.folder
    cv_technique = str(args.cv_technique)
    debug_level = int(args.debuglevel)
    random_state = check_random_state(42)

    result_files = ResultDirectories(folder=folder, debug_level=debug_level)
    setup_logging(log_path=result_files.learning_log_file)
    logger = logging.getLogger("LearningExperiment")
    logger.info("Arguments {}".format(args))
    csv_reader = CSVReader(folder=folder, seed=42)
    csv_reader.plot_class_distribution()
    dataset = args.folder.split('/')[-1]
    if os.path.exists(result_files.accuracies_file):
        with open(result_files.accuracies_file, 'rb') as f:
            metrics_dictionary = pickle.load(f)
        f.close()
    else:
        metrics_dictionary = dict()
    start = datetime.now()
    if cv_technique == 'kfcv':
        cv_iterator = StratifiedKFold(n_splits=cv_iterations, shuffle=True, random_state=random_state)
        if csv_reader.minimum_instances < cv_iterations * 10:
            raise ValueError('Number of instances per class should be greater than {}'.format(cv_iterations * 3))
    elif cv_technique == 'mccv':
        cv_iterator = StratifiedShuffleSplit(n_splits=cv_iterations, test_size=test_size, random_state=random_state)
        if csv_reader.minimum_instances < cv_iterations * 3:
            raise ValueError('Number of instances per class should be greater than {}'.format(cv_iterations * 3))
    elif cv_technique == 'auto':
        if csv_reader.minimum_instances > cv_iterations * 10:
            cv_iterator = StratifiedKFold(n_splits=cv_iterations, shuffle=True, random_state=random_state)
        elif csv_reader.minimum_instances > cv_iterations * 3:
            cv_iterator = StratifiedShuffleSplit(n_splits=cv_iterations, test_size=test_size, random_state=random_state)
        else:
            raise ValueError(f'The number of instances per class ({csv_reader.minimum_instances} in this dataset) should be greater than {cv_iterations * 3}. Try increasing the number of handshakes done by the client.')
    else:
        raise ValueError('Cross-Validation technique is does not exist should be {} or {} or {}'.format(*cv_choices))
    logger.info('The selected Cross-Validation technique is {}'.format(cv_iterator))

    cv_iterations_dict = {}
    cv_iterations_dict[CV_ITERATOR] = str(cv_iterator).split('(')[0]
    cv_iterations_dict[N_SPLITS] = cv_iterations
    metrics_dictionary[DEBUG_LEVEL] = debug_level
    for missing_ccs_fin, (label, j) in product(csv_reader.ccs_fin_array, list(csv_reader.label_mapping.items())):
        start_label = datetime.now()
        dt_string = start_label.strftime("%d/%m/%Y %H:%M:%S")
        logger.info("#############################################################################")
        logger.info("Starting time = {}".format(dt_string))
        i = 0
        x, y = csv_reader.get_data_class_label(class_label=j, missing_ccs_fin=missing_ccs_fin)
        if j == 0:
            label = MULTI_CLASS
            logger.info("Skipping Multi-Class")
            continue
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'

        for classifier, params, search_space in classifiers_space:
            logger.info("#############################################################################")
            logger.info("Classifier {}, running for class {}".format(classifier.__name__, label))
            cls_name = classifier.__name__
            KEY = SCORE_KEY_FORMAT.format(cls_name, label)
            scores_m = metrics_dictionary.get(KEY, None)
            if skip_existing and scores_m is not None:
                logger.info("Classifier {}, is already evaluated for label {}".format(classifier.__name__, label))
                print_accuracies(cls_name, label, scores_m)
            else:
                params['random_state'] = random_state
                n_classes = csv_reader.n_labels
                if int(test_size * y.shape[0]) < n_classes:
                    test_size = (n_classes * 2) / y.shape[0]
                scores_m = optimize_search_cv(classifier, params, search_space, cv_iterator, hp_iterations, x, y,
                                              n_jobs=n_jobs, random_state=random_state)
                metrics_dictionary[KEY] = scores_m
                name = cls_name.lower() + '-' + '_'.join(label.lower().split(' ')) + '.pickle'
                file_name = os.path.join(result_files.models_folder, name)
                idx = np.argmin(np.array(scores_m[BEST_PARAMETERS])[:, 0])
                best_params = scores_m[BEST_PARAMETERS][idx][1]
                params_str = print_dictionary(best_params, sep='\t')
                params.update(best_params)
                best_estimator = classifier(**params)
                best_estimator.fit(x, y)
                with open(file_name, 'wb') as f:
                    pickle.dump(best_estimator, f)
        end_label = datetime.now()
        total = (end_label - start_label).total_seconds()
        logger.info("Time taken for evaluation of label {} is {} minutes ".format(label, total / 60))
        logger.info("#######################################################################")
    end = datetime.now()
    total = (end - start).total_seconds()
    logger.info("Time taken for finishing the learning task is {} seconds and {} hours".format(total, total / 3600))
    logger.info("#######################################################################")
    metrics_dictionary[CV_ITERATIONS_LABEL] = cv_iterations_dict
    with open(result_files.accuracies_file, 'wb') as file:
        pickle.dump(metrics_dictionary, file)
