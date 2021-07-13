import logging
import os
from abc import ABCMeta

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from sklearn.preprocessing import LabelEncoder

from pycsca.utils import print_dictionary

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
        self.data_frame = pd.read_csv(self.df_file, index_col=0)
        self.data_frame['label'] = self.data_frame['label'].apply(lambda x: ' '.join(x.split('_')).title())
        labels = list(self.data_frame['label'].unique())
        labels.sort()
        labels.remove(self.correct_class)

        label_encoder = LabelEncoder()
        label_encoder.fit_transform(labels)
        self.label_mapping = dict(zip(label_encoder.classes_, label_encoder.transform(label_encoder.classes_) + 1))
        self.label_mapping = {**{self.correct_class: 0}, **self.label_mapping}
        self.inverse_label_mapping = dict((v, k) for k, v in self.label_mapping.items())
        self.n_labels = len(self.label_mapping)

        self.data_raw = pd.DataFrame.copy(self.data_frame)
        self.data_frame['label'].replace(self.label_mapping, inplace=True)
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
        if 'missing_ccs_fin' in self.data_frame.columns:
            self.ccs_fin_array = list(self.data_frame['missing_ccs_fin'].unique())

    def plot_class_distribution(self, missing_ccs_fin=False):
        if 'missing_ccs_fin' in self.data_frame.columns:
            df = self.data_frame[self.data_frame['missing_ccs_fin'] == missing_ccs_fin]
        else:
            df = self.data_frame
        df['label'].replace(self.inverse_label_mapping, inplace=True)
        ax = df['label'].value_counts().plot(kind='barh', figsize=(8, 6), title="Label Frequency")
        ax.set_xlabel("Frequency")
        plt.show()

    def get_data_class(self, class_label=1, missing_ccs_fin=False):
        if 'missing_ccs_fin' in self.data_frame.columns:
            df = self.data_frame[self.data_frame['missing_ccs_fin'] == missing_ccs_fin]
        else:
            df = self.data_frame
        if class_label == 0:
            x, y = self.get_data(df)
        else:
            p = [0, class_label]
            df = df[df.label.isin(p)]
            df['label'].replace([class_label], 1, inplace=True)
            x, y = df[self.feature_names].values, df['label'].values.flatten()
        return x, y

    def get_data(self, df):
        x, y = df[self.feature_names].values, df['label'].values.flatten()
        return x, y
