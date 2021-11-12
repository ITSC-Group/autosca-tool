import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from matplotlib import rcParams

from matplotlib.patches import Patch

colors = ['black', 'indigo', 'blueviolet', 'mediumorchid', 'plum', 'mediumblue', 'firebrick',
          'darkorange', 'sandybrown', 'darkgoldenrod', 'gold', 'khaki']


def bar_grid_for_dataset(dataframe_1, title_1, dataframe_2, title_2, dataframe_3, title_3, dataframe_4, title_4,
                         dataframe_5, title_5, output_filename):
    dataframe_1_filtered = dataframe_1[dataframe_1.Dataset.eq('0X00 In Pkcs#1 Padding (First 8 Bytes After 0X00 0X02)')]
    dataframe_2_filtered = dataframe_2[dataframe_2.Dataset.eq('0X00 In Pkcs#1 Padding (First 8 Bytes After 0X00 0X02) Missing-CCS-FIN')]
    dataframe_3_filtered = dataframe_3[dataframe_3.Dataset.eq('0X00 In Pkcs#1 Padding (First 8 Bytes After 0X00 0X02) Missing-CCS-FIN')]
    dataframe_4_filtered = dataframe_4[dataframe_4.Dataset.eq('0X00 In Pkcs#1 Padding (First 8 Bytes After 0X00 0X02)')]
    dataframe_5_filtered = dataframe_5[dataframe_5.Dataset.eq('0X00 In Pkcs#1 Padding (First 8 Bytes After 0X00 0X02)')]

    # Use serif font
    rcParams['font.family'] = 'serif'

    # Create two subplots
    fig, (axis_1, axis_2, axis_3, axis_4, axis_5) = plt.subplots(1, 5, figsize=(6.66, 2.5), sharey=True)

    # Calculate bar locations
    bar_x_positions = []
    bar_width = 0.5
    offset = 0.1
    space = 0.3
    bar_width_offset = bar_width + offset
    for i in [1, 4, 1, 1, 2, 3]:
        if len(bar_x_positions) == 0:
            bar_x_positions.extend(list(np.arange(1, i + 1) * bar_width_offset))
        else:
            ll = (bar_x_positions[-1] + space) + (np.arange(1, i + 1) * bar_width_offset)
            bar_x_positions.extend(ll)

    # Plot the data as bars
    axis_1.bar(x=bar_x_positions, height=dataframe_1_filtered.Accuracy, width=bar_width, color=colors, linewidth=0,
                  label=dataframe_1_filtered.Model)
    axis_2.bar(x=bar_x_positions, height=dataframe_2_filtered.Accuracy, width=bar_width, color=colors, linewidth=0)
    axis_3.bar(x=bar_x_positions, height=dataframe_3_filtered.Accuracy, width=bar_width, color=colors, linewidth=0)
    axis_4.bar(x=bar_x_positions, height=dataframe_4_filtered.Accuracy, width=bar_width, color=colors, linewidth=0)
    axis_5.bar(x=bar_x_positions, height=dataframe_5_filtered.Accuracy, width=bar_width, color=colors, linewidth=0)

    # Create an out-of-plot legend
    #labels = dataframe_1_filtered['Model'].str.replace('Classifier', '')
    #patches = [Patch(color=color, label=label) for color, label in zip(colors, labels)]
    #legend = plt.legend(handles=patches, bbox_to_anchor=(1, -0.15), loc='upper right',
    #                    ncol=1)  # bbox_transform=fig.transFigure,

    # Layout improvements
    fig.tight_layout()
    axis_1.get_xaxis().set_visible(False)
    axis_2.get_xaxis().set_visible(False)
    axis_3.get_xaxis().set_visible(False)
    axis_4.get_xaxis().set_visible(False)
    axis_5.get_xaxis().set_visible(False)
    axis_1.set_ylim((0, 1))
    axis_2.set_ylim((0, 1))
    axis_3.set_ylim((0, 1))
    axis_4.set_ylim((0, 1))
    axis_5.set_ylim((0, 1))
    axis_1.set_ylabel('Accuracy')
    axis_1.set_title(title_1)
    axis_2.set_title(title_2)
    axis_3.set_title(title_3)
    axis_4.set_title(title_4)
    axis_5.set_title(title_5)
    axis_1.axhline(0.5, color='black', linewidth=1, zorder=-1)
    axis_2.axhline(0.5, color='black', linewidth=1, zorder=-1)
    axis_3.axhline(0.5, color='black', linewidth=1, zorder=-1)
    axis_4.axhline(0.5, color='black', linewidth=1, zorder=-1)
    axis_5.axhline(0.5, color='black', linewidth=1, zorder=-1)

    # Save plot to file
    fig.savefig(output_filename, bbox_inches='tight')
    if 'png' in output_filename:
        fig.show()


output_folder = os.path.join(os.getcwd(), 'results')
dataframe_cisco = pd.read_csv('../datasets/2021-01-30-ciscoace/Model Results.csv', index_col=0)
dataframe_f5 = pd.read_csv('../datasets/2021-01-30-f5v1/Model Results.csv', index_col=0)
dataframe_facebook = pd.read_csv('../datasets/2021-02-01-facebookv2/Model Results.csv', index_col=0)
dataframe_netscaler = pd.read_csv('../datasets/2021-01-30-netscalergcm/Model Results.csv', index_col=0)
dataframe_pan = pd.read_csv('../datasets/2021-01-30-panos/Model Results.csv', index_col=0)
bar_grid_for_dataset(dataframe_cisco, 'Cisco ACE',
                     dataframe_f5, 'F5 v1',
                     dataframe_facebook, 'Facebook v2',
                     dataframe_netscaler, 'Netscaler GCM',
                     dataframe_pan, 'PAN OS',
                     './plots/robot_accuracies.png')
