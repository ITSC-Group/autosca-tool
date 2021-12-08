import logging
import os
from abc import ABCMeta

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
import numpy as np
from sklearn.preprocessing import LabelEncoder

from .constants import LABEL_COL, MISSING_CCS_FIN
from .utils import str2bool, print_dictionary

sns.set(color_codes=True)
plt.style.use('default')


class CSVReader(metaclass=ABCMeta):
    def __init__(self, folder: str, preprocessing='replace', **kwargs):
        self.logger = logging.getLogger(CSVReader.__name__)
        self.dataset_folder = folder
        self.f_file = os.path.join(self.dataset_folder, "Feature Names.csv")
        self.df_file = os.path.join(self.dataset_folder, "Features.csv")
        self.preprocessing = preprocessing
        self.ccs_fin_array = [False]
        self.correct_class = "Correctly Formatted Pkcs#1 Pms Message"
        self.__load_dataset__()

    def __load_dataset__(self):
        if not os.path.exists(self.df_file):
            raise ValueError("No such file or directory: {}".format(self.df_file))
        self.data_frame = pd.read_csv(self.df_file, index_col=0)

        if LABEL_COL not in self.data_frame.columns:
            error_string = 'Dataframe does not contain label columns'
            if self.data_frame.shape[0] == 0:
                raise ValueError('Dataframe is empty and {}'.format(error_string))
        else:
            df = pd.DataFrame.copy(self.data_frame)
            df[LABEL_COL] = df[LABEL_COL].apply(lambda x: ' '.join(x.split('_')).title())
            if self.correct_class not in df[LABEL_COL].unique():
                raise ValueError('Dataframe is does not contain correct class {}'.format(self.correct_class))

        self.data_frame[LABEL_COL] = self.data_frame[LABEL_COL].apply(lambda x: ' '.join(x.split('_')).title())
        labels = list(self.data_frame[LABEL_COL].unique())
        labels.sort()
        labels.remove(self.correct_class)

        label_encoder = LabelEncoder()
        label_encoder.fit_transform(labels)
        self.label_mapping = dict(zip(label_encoder.classes_, label_encoder.transform(label_encoder.classes_) + 1))
        self.label_mapping = {**{self.correct_class: 0}, **self.label_mapping}
        self.inverse_label_mapping = dict((v, k) for k, v in self.label_mapping.items())
        self.n_labels = len(self.label_mapping)

        self.data_raw = pd.DataFrame.copy(self.data_frame)
        self.data_frame[LABEL_COL].replace(self.label_mapping, inplace=True)
        self.logger.info("Label Mapping {}".format(print_dictionary(self.label_mapping)))
        self.logger.info("Inverse Label Mapping {}".format(print_dictionary(self.inverse_label_mapping)))

        if self.preprocessing == 'replace':
            self.data_frame = self.data_frame.fillna(value=-1)
        elif self.preprocessing == 'remove':
            cols = [c for c in self.data_frame.columns if 'msg1' not in c or 'msg5' not in c]
            self.data_frame = self.data_frame[cols]
            self.data_frame = self.data_frame.fillna(value=-1)
        self.features = pd.read_csv(self.f_file, index_col=0)
        self.feature_names = self.features['machine'].values.flatten()
        if MISSING_CCS_FIN in self.data_frame.columns:
            self.data_frame[MISSING_CCS_FIN] = self.data_frame[MISSING_CCS_FIN].apply(str2bool)
            self.ccs_fin_array = list(self.data_frame[MISSING_CCS_FIN].unique())
        df = pd.DataFrame.copy(self.data_frame)
        df[LABEL_COL].replace(self.inverse_label_mapping, inplace=True)
        df = pd.DataFrame.copy(self.data_frame)
        df[LABEL_COL].replace(self.inverse_label_mapping, inplace=True)
        df = pd.DataFrame(df[[LABEL_COL, MISSING_CCS_FIN]].value_counts().sort_index())
        df.reset_index(inplace=True)
        df.rename({0: 'Frequency'}, inplace=True, axis='columns')
        df.sort_values(by=[MISSING_CCS_FIN, LABEL_COL], inplace=True)
        f_vals = df.loc[df[LABEL_COL] == self.correct_class][[MISSING_CCS_FIN, 'Frequency']].values
        vals = dict(zip(f_vals[:, 0], f_vals[:, 1]))

        def div(row, val):
            return row['Frequency'] / val

        df['ratio_1_0'] = df.apply(
            lambda row: div(row, vals[True]) if str2bool(row.missing_ccs_fin) else div(row, vals[False]), axis=1)
        fname = os.path.join(self.dataset_folder, "label_frequency.csv")
        df.to_csv(fname)
        self.minimum_instances = np.min(df['Frequency'].values)

    def plot_class_distribution(self):
        fig_param = {'facecolor': 'w', 'edgecolor': 'w', 'transparent': False, 'dpi': 800, 'bbox_inches': 'tight',
                     'pad_inches': 0.05}
        dfs = []
        data_frame = pd.DataFrame.copy(self.data_frame)
        if MISSING_CCS_FIN in data_frame.columns:
            for val in self.ccs_fin_array:
                df = data_frame[data_frame[MISSING_CCS_FIN] == val]
                dfs.append((df, val))
        else:
            # df = pd.DataFrame.copy(data_frame)
            dfs.append((data_frame, 'NA'))
        n_r = len(dfs)
        fig, axs = plt.subplots(nrows=n_r, ncols=1, figsize=(5, 3 * n_r + 2), frameon=True, edgecolor='k',
                                facecolor='white')
        title = ''
        if n_r == 1:
            df, val = dfs[0]
            ax = axs
            df[LABEL_COL].replace(self.inverse_label_mapping, inplace=True)
            d = dict(df[LABEL_COL].value_counts())
            ax.barh(list(d.keys()), list(d.values()), color="r", align="center")
            ax.set_yticks(range(len(d)))
            ax.set_yticklabels(list(d.keys()))
            ax.set_title(title, y=0.95, fontsize=10)
            ax.spines['right'].set_visible(False)
            ax.spines['top'].set_visible(False)
            # ax = df[LABEL_COL].value_counts().plot(kind='barh', title=title)
            ax.set_xlabel("Label Frequency")
        else:
            for (df, val), ax in zip(dfs, axs):
                df[LABEL_COL].replace(self.inverse_label_mapping, inplace=True)
                if MISSING_CCS_FIN in data_frame.columns:
                    title = ' Missing-CCS-FIN ' + str(val)
                d = dict(df[LABEL_COL].value_counts())
                ax.barh(list(d.keys()), list(d.values()), color="r", align="center")
                ax.set_yticks(range(len(d)))
                ax.set_yticklabels(list(d.keys()))
                ax.set_title(title, y=0.95, fontsize=10)
                ax.spines['right'].set_visible(False)
                ax.spines['top'].set_visible(False)
                # ax = df[LABEL_COL].value_counts().plot(kind='barh', title=title)
                ax.set_xlabel("Label Frequency")

        fname = os.path.join(self.dataset_folder, "plot_label_frequency.png")
        fig_param['fname'] = fname
        fig.savefig(**fig_param)

    def get_data_class_label(self, class_label=1, missing_ccs_fin=False):
        if MISSING_CCS_FIN in self.data_frame.columns:
            df = self.data_frame[self.data_frame[MISSING_CCS_FIN] == missing_ccs_fin]
        else:
            df = self.data_frame
        if class_label == 0:
            x, y = self.get_data(df)
        else:
            p = [0, class_label]
            df = df[df.label.isin(p)]
            df[LABEL_COL].replace([class_label], 1, inplace=True)
            x, y = df[self.feature_names].values, df[LABEL_COL].values.flatten()
        return x, y

    def get_data(self, df):
        x, y = df[self.feature_names].values, df[LABEL_COL].values.flatten()
        return x, y
