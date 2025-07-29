import pandas as pd
import matplotlib.pyplot as plt
import tkinter as tk
from tkinter import filedialog

def plot_imu_data(filepath):
    """
    Lê um arquivo CSV de dados do IMU e plota a aceleração e o giroscópio
    em gráficos separados.
    """
    try:
        # Lê os dados usando pandas
        data = pd.read_csv(filepath)
        print("Dados carregados com sucesso!")
        print(data.head())
    except FileNotFoundError:
        print(f"Erro: Arquivo não encontrado em '{filepath}'")
        return
    except Exception as e:
        print(f"Ocorreu um erro ao ler o arquivo: {e}")
        return

    # Verifica se as colunas esperadas existem
    expected_columns = ['numero_amostra', 'accel_x', 'accel_y', 'accel_z', 'giro_x', 'giro_y', 'giro_z']
    if not all(col in data.columns for col in expected_columns):
        print("Erro: O arquivo CSV não contém as colunas esperadas.")
        print(f"Colunas encontradas: {data.columns.tolist()}")
        return

    # --- Gráfico de Aceleração ---
    plt.figure(figsize=(12, 6))
    plt.plot(data['numero_amostra'], data['accel_x'], label='Accel X')
    plt.plot(data['numero_amostra'], data['accel_y'], label='Accel Y')
    plt.plot(data['numero_amostra'], data['accel_z'], label='Accel Z')
    plt.title('Dados do Acelerômetro')
    plt.xlabel('Número da Amostra')
    # ALTERADO: Atualizada a legenda do eixo Y
    plt.ylabel('Aceleração (g)')
    plt.legend()
    plt.grid(True)
    plt.show()

    # --- Gráfico do Giroscópio ---
    plt.figure(figsize=(12, 6))
    plt.plot(data['numero_amostra'], data['giro_x'], label='Giro X')
    plt.plot(data['numero_amostra'], data['giro_y'], label='Giro Y')
    plt.plot(data['numero_amostra'], data['giro_z'], label='Giro Z')
    plt.title('Dados do Giroscópio')
    plt.xlabel('Número da Amostra')
    # ALTERADO: Atualizada a legenda do eixo Y
    plt.ylabel('Velocidade Angular (dps)')
    plt.legend()
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    # Cria uma janela raiz do Tkinter para usar o seletor de arquivos
    root = tk.Tk()
    root.withdraw()  # Oculta a janela principal

    # Abre uma caixa de diálogo para selecionar o arquivo
    file_path = filedialog.askopenfilename(
        title="Selecione o arquivo datalog_xx.csv",
        filetypes=(("CSV files", "*.csv"), ("All files", "*.*"))
    )

    if file_path:
        plot_imu_data(file_path)
    else:
        print("Nenhum arquivo selecionado.")