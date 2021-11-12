import numpy as np
from sklearn.dummy import DummyClassifier
from sklearn.utils import check_random_state

__all__ = ['RandomClassifier', 'MajorityVoting', 'PriorClassifier']


class RandomClassifier(DummyClassifier):
    def __init__(self, **kwargs):
        super(RandomClassifier, self).__init__(strategy='uniform', **kwargs)


class MajorityVoting(DummyClassifier):
    def __init__(self, **kwargs):
        super(MajorityVoting, self).__init__(strategy='most_frequent', **kwargs)


class PriorClassifier(DummyClassifier):
    def __init__(self, random_state=None, **kwargs):
        super(PriorClassifier, self).__init__(strategy='prior', **kwargs)
        self.class_probabilities = [0.5, 0.5]
        self.classes_ = [0, 1]
        self.n_classes = 2
        self.random_state = check_random_state(random_state)

    def fit(self, X, y, sample_weight=None):
        super(PriorClassifier, self).fit(X, y, sample_weight=None)
        self.classes_ = np.unique(y)
        self.n_classes = len(np.unique(y))
        self.class_probabilities = np.zeros(self.n_classes) + 1 / len(self.classes_)
        for i in np.unique(y):
            self.class_probabilities[i] = len(y[y == i]) / len(y)

    def predict(self, X):
        n = X.shape[0]
        return self.random_state.choice(self.classes_, p=self.class_probabilities, size=n)
