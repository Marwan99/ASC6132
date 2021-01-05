import pyabc
import matplotlib.pyplot as plt


history = pyabc.History("sqlite:///" + "calib_data/test.db")

posteriors = history.get_distribution()

print("\nFinal values are:\n")

for i in range(posteriors[0].shape[1]):
    print(posteriors[0].columns[i], "=", posteriors[0].values[0][i])


fig, ax = plt.subplots()
print(history.max_t+1)
for t in range(history.max_t+1):
    df, w = history.get_distribution(m=0, t=t)
    pyabc.visualization.plot_kde_1d(
        df, w,
        xmin=0, xmax=5,
        x="min_death", ax=ax,
        label="PDF t={}".format(t))
ax.axvline(color="k", linestyle="dashed")
ax.legend()