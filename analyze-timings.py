import pandas as pd
import numpy as np

# Read CSV files

# # Load the CSV file into a DataFrame
f6 = 'timings6.csv'
f300 = 'timings300.csv'
f600 = 'timings600.csv'
f900 = 'timings900.csv'

t6 = pd.read_csv(f6)
t300 = pd.read_csv(f300)
t600 = pd.read_csv(f600)
t900 = pd.read_csv(f900)

# Identify the columns to include in the mean calculation
config_columns = [col for col in t6.columns if 'TotalTime' not in col]
config_columns = [col for col in t300.columns if 'TotalTime' not in col]
config_columns = [col for col in t300.columns if 'TotalTime' not in col]
config_columns = [col for col in t600.columns if 'TotalTime' not in col]
config_columns = [col for col in t900.columns if 'TotalTime' not in col]

# Calculate the mean for each row based on the specified columns
t6['runsMean'] = t6[config_columns].mean(axis=1)
t6['totalMean'] = t6['TotalTime']/50
t300['runsMean'] = t300[config_columns].mean(axis=1)
t300['totalMean'] = t300['TotalTime']/50
t600['runsMean'] = t600[config_columns].mean(axis=1)
t600['totalMean'] = t600['TotalTime']/50
t900['runsMean'] = t900[config_columns].mean(axis=1)
t900['totalMean'] = t900['TotalTime']/50

result = pd.DataFrame({
    'Configuration': t900['Configuration'],
    'runsMean6': t6['runsMean'],
    'Total6': t6['TotalTime'],
    'totalMean6': t6['totalMean'],
    'runsMean300': t300['runsMean'],
    'Total300': t300['TotalTime'],
    'totalMean300': t300['totalMean'],
    'runsMean600': t600['runsMean'],
    'Total600': t600['TotalTime'],
    'totalMean600': t600['totalMean'],
    'runsMean900': t900['runsMean'],
    'Total900': t900['TotalTime'],
    'totalMean900': t900['totalMean'],
})

result.replace(r'^\s*$', np.nan, regex=True, inplace=True)

# Display or save the updated DataFrame
print("Mean of timings\n", result)
result.to_csv("mean_results.csv", index=False)
