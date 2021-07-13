import logging
import os
from datetime import datetime

from sklearn.linear_model import Perceptron
from sklearn.model_selection import cross_validate, StratifiedShuffleSplit
from sklearn.svm import LinearSVC
from skopt import BayesSearchCV

from pycsca.utils import print_dictionary
from .baseline import RandomClassifier


def callback(logger):
    def on_step(opt_result):
        """
        Callback meant to view scores after
        each iteration while performing Bayesian
        Optimization in Skopt"""
        points = opt_result.x_iters
        scores = -opt_result.func_vals
        logger.info('Next parameters:\n{}, accuracy {}'.format(points[-1], scores[-1]))

    return on_step


def optimize_search_cv(classifier, params, search_space, test_size, cv_iter, n_iter, x, y):
    logger = logging.getLogger('Classifier-Test')
    clf = classifier(**params)
    # y_pred = None
    # pred_prob = None
    if os.environ.get('OS', '') == 'Windows_NT':
        n_jobs = 1
    else:
        n_jobs = os.cpu_count() - 1
    start = datetime.now()
    dt_string = start.strftime("%d/%m/%Y %H:%M:%S")
    logger.info("Starting time = {}".format(dt_string))
    best_estimator = None
    cv_iterator = StratifiedShuffleSplit(n_splits=cv_iter, test_size=test_size, random_state=42)
    if classifier.__name__ == RandomClassifier.__name__:
        scoring = ['accuracy', 'f1_macro']
        scores = cross_validate(clf, x, y, cv=cv_iterator, scoring=scoring, return_train_score=False)
    else:
        shuffle_split = StratifiedShuffleSplit(n_splits=2, test_size=test_size, random_state=42)
        bayes_search = BayesSearchCV(clf, search_space, n_iter=n_iter, scoring="accuracy", n_jobs=n_jobs,
                                     cv=shuffle_split, error_score=0)  # , optimizer_kwargs={'acq_func': 'EI'})
        try:
            bayes_search.fit(x, y, callback=callback(logger))
            params_str = print_dictionary(bayes_search.best_params_)
            logger.info("Best parameters {}For accuracy of: {:.4f}\n".format(params_str, bayes_search.best_score_))
            model = bayes_search.best_estimator_
        except Exception as err:
            exception_type = type(err).__name__
            logger.info("Exception {}, error {}".format(exception_type, err))
            if bayes_search.best_params_ is not None:
                params.update(bayes_search.best_params_)
                params_str = print_dictionary(bayes_search.best_params_)

                logger.info("Best parameters {}For accuracy of {:.4f}:\n".format(params_str, bayes_search.best_score_))
            if "max_iter" in params.keys():
                params['max_iter'] = 10000
            model = classifier(**params)
        scoring = ['accuracy', 'f1_macro']
        scores = cross_validate(model, x, y, cv=cv_iterator, scoring=scoring,
                                return_train_score=False, return_estimator=True, n_jobs=n_jobs)
        best_estimator = scores['estimator'][0]

        # model = bayes_search.best_estimator_.fit(x, y)
        # y_pred = model.predict(x)
        # try:
        #    pred_prob = model.predict_proba(x)
        # except:
        #    pred_prob = model.decision_function(x)
        # if len(pred_prob.shape) == 2 and pred_prob.shape[-1] > 1:
        #    pred_prob = np.max(pred_prob, axis=1)
        # else:
        #    pred_prob = pred_prob.flatten()

    logger.info("{}, Training time {}, Test time {}".format(type(clf).__name__, scores['fit_time'].sum(),
                                                            scores['score_time'].sum()))
    end = datetime.now()
    total = (end - start).total_seconds() * 1000
    logger.info("Total Time taken by the learner {} is {} milliseconds "
                "and {} seconds".format(type(clf).__name__, total, total / 1000))
    return scores['test_accuracy'], scores['test_f1_macro'], best_estimator
