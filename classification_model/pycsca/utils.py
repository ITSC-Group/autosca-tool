import inspect
import logging
import os
import sys
from sklearn.preprocessing import StandardScaler
import numpy as np

__all__ = ['create_dir_recursively', 'setup_logging', 'progress_bar', 'print_dictionary', 'str2bool',
           'standardize_features', 'standardize_features']


def progress_bar(count, total, status=''):
    bar_len = 60
    filled_len = int(round(bar_len * count / float(total)))

    bar = '=' * filled_len + '-' * (bar_len - filled_len)

    sys.stdout.write('[%s] %s/%s ...%s\r' % (bar, count, total, status))
    sys.stdout.flush()


def print_dictionary(dictionary, sep='\n'):
    output = "\n"
    for key, value in dictionary.items():
        output = output + str(key) + " => " + str(value) + sep
    return output


def create_dir_recursively(path, is_file_path=False):
    if is_file_path:
        path = os.path.dirname(path)
    if not os.path.exists(path):
        os.makedirs(path, exist_ok=True)


def standardize_features(x_train, x_test):
    standardize = Standardize()
    x_train = standardize.fit_transform(x_train)
    x_test = standardize.transform(x_test)
    return x_train, x_test


class Standardize(object):
    def __init__(self, scalar=StandardScaler):
        self.scalar = scalar
        self.n_features = None
        self.scalars = dict()

    def fit(self, X):
        if isinstance(X, dict):
            self.n_features = list(X.keys())
            for k, x in X.items():
                scalar = self.scalar()
                self.scalars[k] = scalar.fit(x)
        if isinstance(X, (np.ndarray, np.generic)):
            self.scalar = self.scalar()
            self.scalar.fit(X)
            self.n_features = X.shape[-1]

    def transform(self, X):
        if isinstance(X, dict):
            for n in self.n_features:
                X[n] = self.scalars[n].transform(X[n])
        if isinstance(X, (np.ndarray, np.generic)):
            X = self.scalar.transform(X)
        return X

    def fit_transform(self, X):
        self.fit(X)
        X = self.transform(X)
        return X


def setup_logging(log_path=None, level=logging.DEBUG):
    """Function setup as many logging for the experiments"""
    if log_path is None:
        dirname = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
        dirname = os.path.dirname(dirname)
        log_path = os.path.join(dirname, "experiments", "logs", "logs.log")
        create_dir_recursively(log_path, True)
    logging.basicConfig(filename=log_path, level=level,
                        format='%(asctime)s %(name)s %(levelname)-8s %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S')
    logger = logging.getLogger("SetupLogger")
    logger.info("log file path: {}".format(log_path))
    logging.getLogger("matplotlib").setLevel(logging.ERROR)


def str2bool(v):
    if int(v) > 0:
        v = 'true'
    return str(v).lower() in ("yes", "true", "t", "1")
