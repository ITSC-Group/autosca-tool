import inspect
import logging
import os
from abc import ABCMeta


class DatasetReader(metaclass=ABCMeta):
    def __init__(self, dataset_folder="", **kwargs):
        """
            The generic dataset parser for parsing datasets for solving different learning problems.

            Parameters
            ----------
            dataset_folder: string
                Name of the folder containing the datasets
            kwargs:
                Keyword arguments for the dataset parser
        """
        self.dr_logger = logging.getLogger(DatasetReader.__name__)
        dirname = os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))
        dirname = os.path.dirname(os.path.dirname(dirname))
        if dataset_folder is not None:
            self.dirname = os.path.join(dirname, dataset_folder)
            if not os.path.exists(self.dirname):
                self.dr_logger.warning("Path given for dataset does not exit {}".format(self.dirname))
        else:
            self.dirname = None

    def __load_dataset__(self):
        raise NotImplementedError
