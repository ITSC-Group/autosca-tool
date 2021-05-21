from sklearn.experimental import enable_hist_gradient_boosting
from sklearn.ensemble import RandomForestClassifier, GradientBoostingClassifier, AdaBoostClassifier, \
    HistGradientBoostingClassifier
from sklearn.linear_model import LogisticRegression, RidgeClassifier, SGDClassifier, Perceptron
from sklearn.svm import LinearSVC
from sklearn.tree import DecisionTreeClassifier, ExtraTreeClassifier
from skopt.space import Categorical, Integer, Real

from .baseline import *

custom_dict = {RandomClassifier.__name__: 0,
               MajorityVoting.__name__: 1,
               PriorClassifier.__name__: 2,
               Perceptron.__name__: 3,
               LogisticRegression.__name__: 4,
               SGDClassifier.__name__: 5,
               RidgeClassifier.__name__: 6,
               LinearSVC.__name__: 7,
               DecisionTreeClassifier.__name__: 8,
               ExtraTreeClassifier.__name__: 9,
               RandomForestClassifier.__name__: 10,
               AdaBoostClassifier.__name__: 11,
               GradientBoostingClassifier.__name__: 12,
               HistGradientBoostingClassifier.__name__: 13
               }
classifiers_space = []

classifiers_space.append((RandomClassifier, {}, {}))
classifiers_space.append((MajorityVoting, {}, {}))
classifiers_space.append((PriorClassifier, {}, {}))

clf = HistGradientBoostingClassifier
params = dict(loss='auto', learning_rate=0.1, max_iter=100, max_leaf_nodes=31, max_depth=None, min_samples_leaf=20,
              l2_regularization=0., max_bins=255, warm_start=False, scoring=None, validation_fraction=0.1,
              n_iter_no_change=5, tol=0.005, early_stopping=True, verbose=0, random_state=None)
search_space = {"learning_rate": Real(5e-2, 1.0, 'log-uniform'),
                "max_depth": Integer(6, 20),
                "max_leaf_nodes": Integer(6, 50),
                "min_samples_leaf": Integer(20, 50),
                "max_bins": Integer(2, 255),
                "l2_regularization": Real(1e-5, 1.0, 'log-uniform')}
# classifiers_space.append((clf, params, search_space))
clf = LinearSVC
params = dict(penalty='l2', loss='hinge', dual=True, tol=1e-4, C=1.0, multi_class='ovr', fit_intercept=True,
              intercept_scaling=1, class_weight=None, verbose=0, random_state=None, max_iter=1000)
search_space = {"tol": Real(1e-4, 1e-3, 'log-uniform'),
                "C": Integer(1, 12),
                "fit_intercept": Categorical([True, False])
                }
classifiers_space.append((clf, params, search_space))

search_space = {"criterion": Categorical(["gini", "entropy"]),
                "bootstrap": Categorical([True, False]),  # values for boostrap can be either True or False
                "max_depth": Integer(6, 20),  # values of max_depth are integers from 6 to 20
                "max_features": Categorical(['auto', 'sqrt', 'log2']),
                "min_samples_leaf": Integer(2, 10),
                "min_samples_split": Integer(2, 10),
                "n_estimators": Integer(50, 300)
                }

clf = RandomForestClassifier
params = dict(bootstrap=True, class_weight=None, criterion='gini', max_depth=14, max_features='sqrt',
              max_leaf_nodes=None, min_impurity_decrease=0.0, min_impurity_split=None, min_samples_leaf=2,
              min_samples_split=4, min_weight_fraction_leaf=0.0, n_estimators=300, n_jobs=None, oob_score=False,
              random_state=None, verbose=0, warm_start=False)
classifiers_space.append((clf, params, search_space))

clf = DecisionTreeClassifier
params = dict(criterion="gini", splitter="best", max_depth=None, min_samples_split=2, min_samples_leaf=1,
              min_weight_fraction_leaf=0., max_features=None, random_state=None, max_leaf_nodes=None,
              min_impurity_decrease=0., min_impurity_split=None, class_weight=None)

search_space = {"criterion": Categorical(["gini", "entropy"]),
                "max_depth": Integer(6, 20),  # values of max_depth are integers from 6 to 20
                "max_features": Categorical(['auto', 'sqrt', 'log2']),
                "min_samples_leaf": Integer(2, 10),
                "min_samples_split": Integer(2, 10),
                "splitter": Categorical(["best", "random"])
                }
classifiers_space.append((clf, params, search_space))

clf = ExtraTreeClassifier
params = dict(criterion="gini", splitter="random", max_depth=None, min_samples_split=2, min_samples_leaf=1,
              min_weight_fraction_leaf=0., max_features="auto", random_state=None, max_leaf_nodes=None,
              min_impurity_decrease=0., min_impurity_split=None, class_weight=None)
classifiers_space.append((clf, params, search_space))

clf = GradientBoostingClassifier
params = dict(loss='deviance', learning_rate=0.1, n_estimators=100, subsample=1.0, criterion='friedman_mse',
              min_samples_split=2, min_samples_leaf=1, min_weight_fraction_leaf=0.0, max_depth=3,
              min_impurity_decrease=0.0, min_impurity_split=None, init=None, n_iter_no_change=5, tol=0.001,
              validation_fraction=0.1, random_state=None, verbose=0)
search_space = {"criterion": Categorical(["friedman_mse", "mse", "mae"]),
                "learning_rate": Real(0.1, 1.0, 'log-uniform'),
                "subsample": Real(0.3, 1.0, 'log-uniform'),
                "max_depth": Integer(6, 20),
                "max_features": Categorical(['auto', 'sqrt', 'log2']),
                "min_samples_leaf": Integer(2, 10),
                "min_samples_split": Integer(2, 10),
                "n_estimators": Integer(50, 300)
                # "loss": Categorical(["deviance", "exponential"]),
                }
classifiers_space.append((clf, params, search_space))

clf = LogisticRegression
params = dict(penalty='l2', dual=False, tol=0.0001, C=1.0, fit_intercept=True, intercept_scaling=1,
              class_weight=None, random_state=None, solver='lbfgs', max_iter=1000, multi_class='auto', verbose=0,
              warm_start=False, n_jobs=None, l1_ratio=None)
search_space = {"tol": Real(1e-5, 0.5, 'log-uniform'), "C": Real(1e-5, 1.0, 'log-uniform'),
                "fit_intercept": Categorical([True, False]),
                "solver": Categorical(['newton-cg', 'lbfgs', 'liblinear', 'sag', 'saga'])
                # "penalty": Categorical(["l1", "l2", "elasticnet", "none"]),
                # "dual": Categorical([True, False]),
                # "l1_ratio": Real(1e-5, 1.0, 'log-uniform'),
                }

classifiers_space.append((clf, params, search_space))

clf = AdaBoostClassifier
params = dict(base_estimator=None, n_estimators=50, learning_rate=1., algorithm='SAMME.R', random_state=None)
search_space = {"n_estimators": Integer(50, 300),
                "learning_rate": Real(5e-2, 1.0, 'log-uniform'),
                "algorithm": Categorical(["SAMME", "SAMME.R"])
                }
classifiers_space.append((clf, params, search_space))

clf = RidgeClassifier
params = dict(alpha=1.0, fit_intercept=True, normalize=False, copy_X=True, max_iter=1000, tol=1e-3, class_weight=None,
              solver="auto", random_state=None)
search_space = {"alpha": Real(0.5, 10, 'log-uniform'),
                "tol": Real(1e-5, 0.5, 'log-uniform'),
                "normalize": Categorical([True, False]),
                "fit_intercept": Categorical([True, False]),
                "solver": Categorical(['auto', 'svd', 'cholesky', 'lsqr', 'sparse_cg', 'sag', 'saga'])}
classifiers_space.append((clf, params, search_space))

clf = SGDClassifier
params = dict(loss="hinge", penalty='l2', alpha=0.0001, l1_ratio=0.15, fit_intercept=True, max_iter=1000, tol=1e-3,
              shuffle=True, verbose=0, epsilon=0.1, n_jobs=None, random_state=None, learning_rate="optimal", eta0=1e-3,
              power_t=0.5, early_stopping=False, validation_fraction=0.1, n_iter_no_change=5, class_weight=None,
              warm_start=False, average=False)
search_space = {"loss": Categorical(['hinge', 'log', 'modified_huber', 'squared_hinge', 'perceptron']),
                "penalty": Categorical(['l1', 'l2', 'elasticnet']),
                "alpha": Real(1e-5, 0.5, 'log-uniform'),
                "l1_ratio": Real(0.01, 1.0, 'log-uniform'),
                "fit_intercept": Categorical([True, False]),
                "tol": Real(1e-4, 5e-1, 'log-uniform'),
                "epsilon": Real(1e-4, 5e-1, 'log-uniform'),
                "learning_rate": Categorical(['constant', 'optimal', 'invscaling', 'adaptive']),
                'average': Categorical([True, False])
                }
classifiers_space.append((clf, params, search_space))

clf = Perceptron
params = dict(penalty='l2', alpha=0.0001, fit_intercept=True, max_iter=1000, tol=1e-3,
              early_stopping=False, class_weight=None)
search_space = {"penalty": Categorical(['l1', 'l2', 'elasticnet']),
                "alpha": Real(1e-5, 0.5, 'log-uniform'),
                "fit_intercept": Categorical([True, False]),
                "tol": Real(1e-4, 5e-1, 'log-uniform')
                }
classifiers_space.append((clf, params, search_space))
