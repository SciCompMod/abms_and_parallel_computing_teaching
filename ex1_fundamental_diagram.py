import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV file
vmax = 5
df = pd.read_csv("results_fundamental_diagram_vmax"+str(vmax)+"_L1000.csv")

# Plot
plt.figure(figsize=(8, 5))
for prob in df["probability"].unique():
    subset = df[df["probability"] == prob]
    plt.plot(subset["density"], subset["flow"], marker='x', linestyle='None', label=f'Probability p={prob}')
plt.xlabel("Density", fontsize=14)
plt.ylabel("Flow", fontsize=14)
plt.title("Fundamental Diagram", fontsize=18)
plt.legend(fontsize=14)
plt.grid(True)
plt.tight_layout()
plt.savefig("fundamental_diagram_vmax"+str(vmax)+".png", dpi=300)
plt.show()