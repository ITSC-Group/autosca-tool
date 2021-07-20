import numpy as np
from sklearn.utils import check_random_state


class RandomClassifier(object):
    def __init__(self, random_state=None, **kwargs):
        self.random_state = check_random_state(random_state)
        self.n_classes = 2

    def fit(self, x, y):
        self.n_classes = np.unique(y)

    def predict(self, x):
        n_instances = x.shape[0]
        return self.random_state.choice(self.n_classes, n_instances)

    def get_params(self, **kwargs):
        return {'random_state': self.random_state, 'n_classes': self.n_classes}
