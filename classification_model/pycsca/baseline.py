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
        self.class_probability = 0.5
        self.random_state = check_random_state(random_state)

    def fit(self, X, y, sample_weight=None):
        super(PriorClassifier, self).fit(X, y, sample_weight=None)
        self.class_probability = np.mean(y)

    def predict(self, X):
        n = X.shape[0]
        return self.random_state.choice([0, 1], p=[1 - self.class_probability, self.class_probability], size=n)
