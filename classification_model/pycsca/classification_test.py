import copy
import logging
from datetime import datetime

import numpy as np
from sklearn.dummy import DummyClassifier
from sklearn.metrics import confusion_matrix, roc_auc_score, f1_score, accuracy_score, cohen_kappa_score, \
    matthews_corrcoef
from sklearn.model_selection import StratifiedKFold, train_test_split
from skopt import BayesSearchCV

from pycsca.utils import *
from .constants import *


def instance_informedness(y_true, y_pred):
    tp = np.logical_and(y_true, y_pred).sum()
    tn = np.logical_and(np.logical_not(y_true), np.logical_not(y_pred)).sum()
    cp = np.array(y_true).sum()
    cn = np.logical_not(y_true).sum()
    inf = np.nansum([tp / cp, tn / cn, -1])
    return (inf + 1) / 2


def callback(logger):
    def on_step(opt_result):
        """
        Callback meant to view scores after
        each iteration while performing Bayesian
        Optimization in Skopt"""
        points = opt_result.x_iters
        scores = -opt_result.func_vals
        #logger.info('Next parameters:
        # {}, accuracy {}'.format(points[-1], scores[-1]))

    return on_step


def get_scores(X_test, model):
    y_pred = model.predict(X_test)
    try:
        pred_prob = model.predict_proba(X_test)
    except:
        pred_prob = model.decision_function(X_test)
    # logger.info("Predict Probability shape {}, {}".format(pred_prob.shape, y_test.shape))
    if len(pred_prob.shape) == 2 and pred_prob.shape[-1] > 1:
        if pred_prob.shape[-1] == 2:
            p_pred = pred_prob[:, 1]
        else:
            p_pred = np.max(pred_prob, axis=1)
    else:
        p_pred = pred_prob.flatten()
    return p_pred, y_pred


def get_evaluation(metric_array, metric, y_test, y_pred, logger, i):
    try:
        val = metric(y_test, y_pred)
        if metric == confusion_matrix:
            if val.ravel().shape[0] != 4:
                raise ValueError("Confusion Matrix {}, shape {}".format(val, val.ravel().shape[0]))
        metric_array.append(val)
    except Exception as e:
        logger.error('Could not calculate {} for iteration {} due to error {}'.format(metric.__name__, i, str(e)))
        logger.error((y_test, y_pred))
    return metric_array


def optimize_search_cv(classifier, params, search_space, cv_iterator, hp_iterations, x, y, random_state=42):
    logger = logging.getLogger('Classifier-Test')
    # clf = classifier(**params)
    # y_pred = None
    # pred_prob = None
    n_jobs = 6
    start = datetime.now()
    dt_string = start.strftime("%d/%m/%Y %H:%M:%S")
    logger.info("Starting time = {}".format(dt_string))
    inner_cv_iterator = StratifiedKFold(n_splits=3, shuffle=True, random_state=random_state)
    confusion_matrices = []
    aucs = []
    accuracies = []
    f1_scores = []
    cohen_kappa = []
    mccs = []
    infs = []
    best_estimator = None
    min_acc = -100
    scores = {}
    logger.info("Current default parameters {}".format(print_dictionary(params, sep='\t')))

    for i, (train_index, test_index) in enumerate(cv_iterator.split(x, y)):
        X_train, X_test = x[train_index], x[test_index]
        y_train, y_test = y[train_index], y[test_index]
        model = classifier(**params)
        if issubclass(classifier, DummyClassifier):
            model.fit(X_train, y_train)
            best_estimator = classifier(**params)
        else:
            logger.info(
                "############################## Starting the iteration {} ##############################".format(i + 1))
            bayes_search = BayesSearchCV(model, search_space, n_iter=hp_iterations, scoring="accuracy", n_jobs=n_jobs,
                                         cv=inner_cv_iterator, error_score=0, random_state=random_state)
            try:
                bayes_search.fit(X_train, y_train, callback=callback(logger))
                params = update_params(bayes_search, i, logger, params)
                logger.info("Optimizer Iterations done: {}".format(len(bayes_search.cv_results_['params'])))
            except Exception as err:
                exception_type = type(err).__name__
                logger.info("Exception {}, error {}".format(exception_type, err))
                if bayes_search.best_params_ is not None:
                    params = update_params(bayes_search, i, logger, params)
                    logger.info("Optimizer Iterations done {}".format(len(bayes_search.cv_results_['params'])))

            model = classifier(**params)
            model.fit(X_train, y_train)
            if bayes_search.best_params_ is not None:
                if min_acc < bayes_search.best_score_:
                    min_acc = bayes_search.best_score_
                    best_params = bayes_search.best_params_
                    params.update(best_params)
                    best_estimator = classifier(**params)
                    logger.info("Updating best parameters for the classifier")
        p_pred, y_pred = get_scores(X_test, model)
        confusion_matrices = get_evaluation(confusion_matrices, confusion_matrix, y_test, y_pred, logger, i)
        f1_scores = get_evaluation(f1_scores, f1_score, y_test, y_pred, logger, i)
        accuracies = get_evaluation(accuracies, accuracy_score, y_test, y_pred, logger, i)
        cohen_kappa = get_evaluation(cohen_kappa, cohen_kappa_score, y_test, y_pred, logger, i)
        aucs = get_evaluation(aucs, roc_auc_score, y_test, p_pred, logger, i)
        mccs = get_evaluation(mccs, matthews_corrcoef, y_test, y_pred, logger, i)
        infs = get_evaluation(infs, instance_informedness, y_test, y_pred, logger, i)
    X_train, X_test, y_train, y_test = train_test_split(x, y, test_size=0.50, random_state=42)
    model = copy.deepcopy(best_estimator)
    model.fit(X_train, y_train)
    y_pred = model.predict(X_test)
    cm_single = confusion_matrix(y_test, y_pred)

    best_estimator.fit(x, y)
    end = datetime.now()
    total = (end - start).total_seconds()
    logger.info("Total Time taken by the learner {} is {} seconds "
                "and {} minutes".format(type(best_estimator).__name__, total, total / 60))
    scores[ACCURACY] = np.array(accuracies)
    scores[CONFUSION_MATRIX_SINGLE] = cm_single
    scores[CONFUSION_MATRICES] = np.array(confusion_matrices)
    scores[F1SCORE] = np.array(f1_scores)
    scores[AUC_SCORE] = np.array(aucs)
    scores[COHENKAPPA] = np.array(cohen_kappa)
    scores[MCC] = np.array(mccs)
    scores[INFORMEDNESS] = np.array(infs)
    return scores, best_estimator


def update_params(bayes_search, i, logger, params):
    params.update(bayes_search.best_params_)
    params_str = print_dictionary(bayes_search.best_params_, sep='\t')
    best_score = bayes_search.best_score_
    logger.info("For the outer split {} Best parameters are: {} with accuracy of: {:.4f}\n".format(i + 1, params_str,
                                                                                                   best_score))
    return params
