import glob
import logging
import math
import os
import pickle
import re
from itertools import product

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
from sklearn.model_selection import ShuffleSplit, learning_curve

from .classifiers import custom_dict
from .utils import progress_bar

RANDOM_FOREST_CLASSIFIER = 'RandomForestClassifier'

sns.set(color_codes=True)
plt.style.use('default')

__all__ = ['fig_param', 'colors', 'bar_grid_for_dataset', 'classwise_barplot_for_dataset',
           'bar_plot_for_problem', 'plot_learning_curves_importances', 'pgf_with_latex']

colors = ['black', 'black', 'black', 'indigo', 'blueviolet', 'mediumorchid', 'plum', 'mediumblue', 'firebrick',
          'darkorange', 'sandybrown', 'darkgoldenrod', 'gold', 'khaki']

pgf_with_latex = {  # setup matplotlib to use latex for output
    "pgf.texsystem": "pdflatex",  # change this if using xetex or lautex
    "text.usetex": True,  # use LaTeX to write all text
    "font.family": "serif",
    "font.serif": [],  # blank entries should cause plots
    "font.sans-serif": ['Times New Roman'] + plt.rcParams['font.serif'],  # to inherit fonts from the document
    "font.monospace": [],
    "font.size": 11,
    "legend.fontsize": 11,  # Make the legend/label fonts
    "xtick.labelsize": 11,  # a little smaller
    "ytick.labelsize": 11,
    'pgf.rcfonts': False,
    "pgf.preamble": [
        r"\usepackage[utf8x]{inputenc}",  # use utf8 fonts
        r"\usepackage[T1]{fontenc}",  # plots will be generated
        r"\usepackage[detect-all,locale=DE]{siunitx}",
    ]  # using this preamble
}

fig_param = {'facecolor': 'w', 'edgecolor': 'w', 'transparent': False, 'dpi': 800, 'bbox_inches': 'tight',
             'pad_inches': 0.05}


def init_plots(df, extension, metric, figsize):
    sns.set(color_codes=True)
    plt.style.use('default')
    fig_param['format'] = extension
    if extension == 'pgf':
        plt.rc('text', usetex=True)
        mpl.use('pgf')
        pgf_with_latex["figure.figsize"] = figsize
        mpl.rcParams.update(pgf_with_latex)
    bar_width = 0.5
    opacity = 0.7
    offset = 0.1
    df = df[~df['Dataset'].str.contains('Multi-Class')]
    df['rank'] = df['Model'].map(custom_dict)
    df.sort_values(by='rank', inplace=True)
    del df['rank']
    u_models = list(df.Model.unique())
    u_models = [model.split('Classifier')[0] for model in u_models]
    u_models[u_models.index('SGD')] = "PerceptronLearningAlgorithm"
    u_models[u_models.index('LinearSVC')] = "SupportVectorMachine"
    u_models[u_models.index('Ridge')] = "RidgeClassificationModel"
    u_models = [' '.join(re.findall('[A-Z][^A-Z]*', model)) for model in u_models]
    u_models[0] = u_models[0] + ' Guesser (Baseline)'
    u_datasets = list(df.Dataset.unique())
    bar_width_offset = bar_width + offset
    space = 0.3
    index = []
    for i in [3, 3, 1, 2, 2, 2]:
        if len(index) == 0:
            index.extend(list(np.arange(1, i + 1) * bar_width_offset))
        else:
            ll = (index[-1] + space) + (np.arange(1, i + 1) * bar_width_offset)
            index.extend(ll)
    # j = 0
    if 'pval' in metric:
        end = 0.011
    else:
        end = 1.1
    return bar_width, df, fig_param, index, opacity, u_datasets, u_models, end


def bar_grid_for_dataset(df, metric, std, folder, figsize=(7, 4), extension='png', logger=None):
    bar_width, df, fig_param, index, opacity, u_datasets, u_models, end = init_plots(df, extension, figsize, metric)
    if len(u_datasets) % 4 == 0:
        c = 4
        r = int(len(u_datasets) / c)
    elif len(u_datasets) % 3 == 0:
        c = 3
        r = int(len(u_datasets) / c)
    else:
        c = 5
        r = int(len(u_datasets) / c) + 1
    figsize = (figsize[0] * r, figsize[1] * c)
    logger.info('Datasets {}, figsize {}, rows {}, cols {}'.format(len(u_datasets), figsize, r, c))
    fig, axs = plt.subplots(nrows=r, ncols=c, sharex=True, sharey=True, figsize=figsize, frameon=True, edgecolor='k',
                            facecolor='white')
    plt.figure()
    axs = np.array(axs).flatten()
    ini = index[0]

    for ax, dataset in zip(axs, u_datasets):
        logger.error("Plotting grid plot for dataset {}".format(dataset))
        accs = list(df[df['Dataset'] == dataset][metric].values)
        errors = list(df[df['Dataset'] == dataset][metric + '-std'].values / std)
        ax.bar(x=index, height=accs, yerr=errors, width=bar_width, alpha=opacity, color=colors, tick_label=u_models)
        ax.plot([ini - bar_width / 2, index[-1] + bar_width / 2], [0.5, 0.5], "k--")
        ax.plot([ini - bar_width / 2, index[-1] + bar_width / 2], [1.0, 1.0], "k--")
        ax.set_yticks(np.arange(0, end, step=0.1).round(1))

        l = int(len(dataset.split(" ")) / 2) + 1
        dataset = '_'.join(dataset.split(" ")[0:l]) + '\n' + '_'.join(dataset.split(" ")[l:])
        ax.set_title(dataset, fontsize=10)
        ax.spines['right'].set_visible(False)
        ax.spines['top'].set_visible(False)
        ax.tick_params(labelsize=10)
        ax.tick_params(axis='x', which='major', labelsize=10)
        ax.set_xticklabels(ax.xaxis.get_majorticklabels(), rotation=45, ha='right')
        ax.set_ylim(0, end)
        ax.set_ylabel(metric.title(), fontsize=10)
    fname = os.path.join(folder, "plot_{}.{}".format('grid', extension))
    fig_param['fname'] = fname
    fig.savefig(**fig_param)
    plt.show()


def classwise_barplot_for_dataset(df, metric, std, folder, figsize=(4, 4), extension='png',
                                  logger=logging.getLogger('None')):
    bar_width, df, fig_param, index, opacity, u_datasets, u_models, end = init_plots(df, extension, figsize, metric)
    ini = index[0]
    for dataset in u_datasets:
        logger.error("Plotting single for dataset {}".format(dataset))
        fig, ax = plt.subplots(figsize=figsize, frameon=True, edgecolor='k', facecolor='white')
        accs = list(df[df['Dataset'] == dataset][metric].values)
        errors = list(df[df['Dataset'] == dataset][metric + '-std'].values / std)
        ax.bar(x=index, height=accs, yerr=errors, width=bar_width, alpha=opacity, color=colors, tick_label=u_models)
        ax.plot([ini - bar_width / 2, index[-1] + bar_width / 2], [0.5, 0.5], "k--")
        ax.plot([ini - bar_width / 2, index[-1] + bar_width / 2], [1.0, 1.0], "k--")
        ax.set_yticks(np.arange(0, end, step=0.1).round(1))
        plt.title(dataset, fontsize=10)
        ax.spines['right'].set_visible(False)
        ax.spines['top'].set_visible(False)
        ax.tick_params(labelsize=10)
        ax.tick_params(axis='x', which='major', labelsize=8)
        ax.set_xticklabels(ax.xaxis.get_majorticklabels(), rotation=45, ha='right')
        ax.set_ylim(0, end)

        ax.set_ylabel(metric.title(), fontsize=10)
        plt.tight_layout()
        dataset = re.sub(r'\s*\d+\s*', '', dataset)
        dataset = dataset.replace(" ", "_")
        fname = os.path.join(folder, "plot_{}.{}".format(dataset.lower(), extension))
        fig_param['fname'] = fname
        plt.savefig(**fig_param)


def bar_plot_for_problem(df, metric, params, std, folder, figsize=(14, 6), extension='png',
                         logger=logging.getLogger('None')):
    bar_width, df, fig_param, index, opacity, u_datasets, u_models, end = init_plots(df, extension, figsize, metric)
    init_index = index
    ini = init_index[0]
    fig, ax = plt.subplots(figsize=figsize, frameon=True, edgecolor='k', facecolor='white')
    for model in u_models:
        accs = list(df[df['Model'] == model][metric].values)
        errors = list(df[df['Model'] == model][metric + '-std'].values / std)
        ax.bar(x=index, height=accs, yerr=errors, width=bar_width, alpha=opacity, label=model)
        index = index + bar_width
    end = 1.1
    ax.plot([ini - bar_width, index[-1] - bar_width / 2], [0.5, 0.5], "k--")
    ax.plot([ini - bar_width, index[-1] - bar_width / 2], [1.0, 1.0], "k--")
    ax.set_yticks(np.arange(0, end, step=0.1).round(1))
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.tick_params(labelsize=10)
    ax.set_xticks(init_index)
    ax.set_xticklabels(np.arange(len(u_datasets)) + 1, rotation=0)
    ax.set_ylim(0, end)
    ax.set_ylabel(metric.title(), fontsize=15)
    plt.legend(**params)
    plt.tight_layout()
    fname = os.path.join(folder, "plot_{}.{}".format(metric.lower(), extension))
    fig_param['fname'] = fname
    plt.savefig(**fig_param)


def plot_importance(model1, model2, class_label, feature_names, folder, extension, figsize=(7, 4), number=15):
    fig_param['format'] = extension

    def norm(x):
        return (x - x.min()) / (x.max() - x.min())

    def get_importances(model):
        feature_importances = model.feature_importances_
        trees = np.array(model.estimators_)
        trees = trees.flatten()
        deviation = []
        for tree in trees:
            imp = tree.feature_importances_
            imp = norm(imp)
            deviation.append(imp)
        std = np.std(deviation, axis=0) / len(trees)
        indices = np.argsort(feature_importances)[::-1]

        importances = norm(feature_importances)[indices][0:number]
        names = feature_names[indices[0:number]]
        std = std[indices][0:number]
        return importances, std, names

    importances, std, names = get_importances(model1)
    importances2, std2, names2 = get_importances(model2)

    fig, (ax1, ax2) = plt.subplots(nrows=2, ncols=1, figsize=figsize, frameon=True, edgecolor='k', facecolor='white')
    if '(' in class_label:
        l = class_label.index('(')
        class_label = ' '.join(class_label[0:l].split(" ")) + '\n' + ' '.join(class_label[l:].split(" ")) + '\n'
    else:
        l = int(len(class_label.split(" ")) / 2) + 1
        class_label = ' '.join(class_label.split(" ")[0:l]) + '\n' + ' '.join(class_label.split(" ")[l:]) + '\n'
    ax1.barh(range(number), importances, color="r", xerr=std, align="center")
    ax1.set_yticklabels(names)
    ax1.set_yticks(range(number))
    ax1.spines['right'].set_visible(False)
    ax1.spines['top'].set_visible(False)

    ax2.barh(range(number), importances2, color="r", xerr=std2, align="center")
    ax2.set_yticks(range(number))
    ax2.set_yticklabels(names2)
    ax2.set_title('Missing-CCS-FIN', y=1.01)
    ax2.spines['right'].set_visible(False)
    ax2.spines['top'].set_visible(False)

    fig.suptitle(class_label, y=0.90, fontsize=11)
    class_label = '_'.join(class_label.lower().split(' '))
    class_label = class_label.replace('\n', '')
    fname = os.path.join(folder, "importance_{}.{}".format(class_label, extension))
    fig_param['fname'] = fname
    plt.savefig(**fig_param)


def learning_curve_for_label(estimators, X, y, vulnerable, fname, extension):
    ncols = 2
    nrows = int(len(estimators) / ncols)
    figsize = (7, nrows * 4)
    fig_param['format'] = extension
    fig, axs = plt.subplots(nrows=nrows, ncols=ncols, sharex=True, sharey=True, figsize=figsize,
                            frameon=True, edgecolor='k', facecolor='white')
    axs = np.array(axs).flatten()
    test_size = 0.5
    if not vulnerable:
        cv = ShuffleSplit(n_splits=10, test_size=0.1, random_state=0)
    else:
        cv = ShuffleSplit(n_splits=10, test_size=0.5, random_state=0)
    i = 1
    for ax, estimator in zip(axs, estimators):
        label = type(estimator).__name__
        progress_bar(i, len(estimators), label)
        i += 1
        if not vulnerable:
            train_sizes = np.linspace(0.4, 1.0, num=15)
        else:
            train_sizes = np.arange(10, 300, 10) / X.shape[0]
        train_sizes, train_scores, test_scores, fit_times, _ = learning_curve(estimator, X, y, cv=cv,
                                                                              n_jobs=os.cpu_count() - 2,
                                                                              train_sizes=train_sizes,
                                                                              return_times=True)

        train_sizes = [int(math.ceil(i / 5.0)) * 5 for i in train_sizes]
        train_scores_mean = np.mean(train_scores, axis=1)
        train_scores_std = np.std(train_scores, axis=1)
        test_scores_mean = np.mean(test_scores, axis=1)
        test_scores_std = np.std(test_scores, axis=1)
        fit_times_mean = np.mean(fit_times, axis=1)
        fit_times_std = np.std(fit_times, axis=1)

        # Plot learning curve
        label = label.split('Classifier')[0]
        if 'SGD' in label:
            label = "StochasticGradient\nDescent"
        if 'LinearSVC' in label:
            label = "SupportVector\nMachine"
        if 'Ridge' in label:
            label = "RidgeClassification\nModel"
        if 'Hist' in label:
            label = "HistogramGradient\nBoosting"
        label = ' '.join(re.findall('[A-Z][^A-Z]*', label))
        ax.set_title(label, y=0.90, fontsize=13)
        ax.fill_between(train_sizes, train_scores_mean - train_scores_std,
                        train_scores_mean + train_scores_std, alpha=0.1, color='r')
        ax.fill_between(train_sizes, test_scores_mean - test_scores_std,
                        test_scores_mean + test_scores_std, alpha=0.1, color='k')

        ax.plot(train_sizes, train_scores_mean, 'o-', label="In-Sample Accuracy", markersize=3, color='r')
        ax.plot(train_sizes, test_scores_mean, 's-', label="Out-of-Sample Accuracy", markersize=3, color='k')

        ax.spines['right'].set_visible(False)
        ax.spines['top'].set_visible(False)
        ax.set_ylim(0.20, 1.20)
        ax.set_yticks(np.linspace(0.3, 1.0, num=8).round(1))
        ax.set_yticklabels(np.linspace(0.3, 1.0, num=8).round(1), fontsize=13)

        diff = train_sizes[1] - train_sizes[0]
        if vulnerable:
            diff = diff * 2
        ax.set_xlim(train_sizes[0] - diff, train_sizes[-1] + diff)

        if vulnerable:
            train_sizes.append(train_sizes[-1] + 5)
            train_sizes.insert(0, train_sizes[0] - 5)
        ax.set_xticks(train_sizes[::2])
        ax.set_xticklabels(train_sizes[::2], rotation=90, ha='right', fontsize=11)

    axs[9].set_xlabel('# Training Examples', x=-0.2, fontsize=14)
    axs[4].set_ylabel('Accuracy', fontsize=14)

    params = dict(loc='lower right', bbox_to_anchor=(1.0, -0.45), ncol=2, fancybox=False, shadow=True,
                  facecolor='white', edgecolor='k', fontsize=13)
    if not vulnerable:
        params['bbox_to_anchor'] = (1.00, -0.45)
    plt.legend(**params)

    fig_param['fname'] = fname
    plt.savefig(**fig_param)


def plot_learning_curves_importances(result_dirs, csv_reader, vulnerable_classes, extension='png', plotlr=False,
                                     logger=logging.getLogger('None')):
    def get_estimators_data(models_folder, csv_reader, label_number, missing_ccs_fin):
        X, y = csv_reader.get_data_class_label(class_label=label_number, missing_ccs_fin=missing_ccs_fin)
        label = csv_reader.inverse_label_mapping[label_number]
        if missing_ccs_fin:
            label = label + ' Missing-CCS-FIN'

        name = '-' + '_'.join(label.lower().split(' ')) + '.pickle'
        estimators = []
        files = glob.glob(os.path.join(models_folder, '*.pickle'))
        files.sort()
        for pic in files:
            if name in pic and not ('randomclassifier' in pic or 'perceptron' in pic):
                with open(pic, 'rb') as f:
                    model = pickle.load(f)
                    estimators.append(model)
        return label, estimators, X, y
    for missing_ccs_fin, (label, label_number) in product(csv_reader.ccs_fin_array,
                                                          list(csv_reader.label_mapping.items())):
        label, estimators, X, y = get_estimators_data(result_dirs.models_folder, csv_reader, label_number=label_number,
                                                      missing_ccs_fin=missing_ccs_fin)
        vulnerable = label in vulnerable_classes
        if vulnerable and not missing_ccs_fin:
            name = RANDOM_FOREST_CLASSIFIER.lower() + '-' + '_'.join(label.lower().split(' ')) + '.pickle'
            f = open(os.path.join(result_dirs.models_folder, name), 'rb')
            model1 = pickle.load(f)
            model1.n_estimators = 500
            model1.fit(X, y)
            label1 = label + ' Missing-CCS-FIN'

            name = RANDOM_FOREST_CLASSIFIER.lower() + '-' + '_'.join(label1.lower().split(' ')) + '.pickle'
            f = open(os.path.join(result_dirs.models_folder, name), 'rb')
            model2 = pickle.load(f)
            model2.n_estimators = 500
            X1, y1 = csv_reader.get_data_class(class_label=label_number, missing_ccs_fin=True)
            model2.fit(X1, y1)

            plot_importance(model1, model2, label, csv_reader.feature_names, result_dirs.importance_folder, extension=extension,
                            figsize=(7, 9), number=15)
        if len(estimators) != 0 and plotlr:
            label = label.replace(" ", "_")
            fname = os.path.join(result_dirs.learning_curves_folder, "learning_curves_{}.{}".format(label, extension))
            learning_curve_for_label(estimators, X, y, vulnerable, fname, extension)
        logger.error("###########################{}-{}###########################".format(vulnerable, label))
