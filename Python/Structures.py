import numpy as np

# Implementacje struktur obliczeniowych filtrów w python

# Implementacja struktury bezpośredniej I (Direct Form I)
def DF1(x, B, A):
  """
  Direct Form I
  x - Sygnał wejściowy
  B - Współczynniki licznika
  A - Współczynniki mianownika
  return - sygnał wyjściowy
  """
  y = np.zeros(x.size)

  for n in range (0, x.size):
    y[n] = 0
    for i in range (0, len(B)):
      #if n - i >= 0:
        y[n] += B[i] / A[0] * x[n - i]

    for i in range (1, len(A)):
      #if n - i >= 0:
        y[n] -= A[i] / A[0] * y[n - i]

  return y

# Implementacja struktury bezpośredniej II (Direct Form II)
def DF2(x, B, A):
  """
  Direct Form II
  x - Sygnał wejściowy
  B - Współczynniki licznika
  A - Współczynniki mianownika
  w - bufor stanu
  return - sygnał wyjściowy
  """
  y = np.zeros(x.size)
  w = np.zeros(x.size)

  for n in range (0, x.size):
    for i in range (1, len(A)):
      w[n] -= A[i] / A[0] * w[n - i]

    w[n] += x[n]
    for i in range(0, len(B)):
      y[n] += B[i] / A[0] * w[n - i]

  return y

def TDF2(x, B, A):
  """
  Transposed Direct Form II (TDF2)
  x - Sygnał wejściowy
  B - Współczynniki licznika
  A - Współczynniki mianownika
  return - sygnał wyjściowy
  """
  y = np.zeros_like(x, dtype=np.float64)
  N = len(B)
  w = np.zeros(N - 1, dtype=np.float64)  # Bufor stanu

  for n in range(x.size):
    # Wyjście filtra
    y[n] = B[0] * x[n] + w[0]

    # Aktualizacja stanu
    for i in range(0, N - 2):
      w[i] = w[i + 1] + B[i + 1] * x[n] - A[i + 1] * y[n]

    w[N - 2] = B[N - 1] * x[n] - A[N - 1] * y[n]

  return y

''' -----Stara implementacja TDF2 (Niestabilna)-----
# Implementacja struktury bezpośredniej transponowanej II (Transposed Direct Form II)
def TDF2(x, B, A):
  """
  Transposed Direct Form II
  x - Sygnał wejściowy
  B - Współczynniki licznika
  A - Współczynniki mianownika
  return - sygnał wyjściowy
  """
  y = np.zeros(x.size)
  w = np.zeros(len(A) - 1)  # Bufor stanu (rząd filtra - 1)

  for n in range(x.size):
    # Oblicz nowy stan wn
    wn = x[n]
    for i in range(1, len(A)):
      wn -= (A[i] / A[0]) * w[i-1]

      # Oblicz wyjście y[n]
      y[n] = (B[0] / A[0]) * wn
      for i in range(1, len(B)):
        y[n] += (B[i] / A[0]) * w[i-1]

      # Przesuń wartości w buforze stanu (FIFO)
      for i in range(len(w) - 1, 0, -1):
        w[i] = w[i-1]
        if len(w) > 0:
          w[0] = wn  # Aktualizacja pierwszego elementu

  return y
'''

# Implementacja struktury kaskadowej sekcji drugiego rzędu (Cascade Form - second order sections)
# konieczna zamiana współczynników na definicje sekcji drugiego rzędu: tf2sos(b, a)
def CASCADE(x, sos_sections):
  """
  Struktura kaskadowa (Second-Order Sections)
  x - Sygnał wejściowy
  sos_sections - Lista sekcji drugiego rzędu [[b0, b1, b2, a0, a1, a2], ...]
  return - sygnał wyjściowy
  """

  y = np.copy(x)

  for section in sos_sections:
    B = section[:3]  # Współczynniki licznika [b0, b1, b2]
    A = section[3:]  # Współczynniki mianownika [a0, a1, a2]

    w = np.zeros(2)  # Bufor stanu dla każdej sekcji

    for n in range(x.size):
      wn = y[n] - (A[1]/A[0]) * w[0] - (A[2]/A[0]) * w[1]

      y[n] = (B[0]/A[0]) * wn + (B[1]/A[0]) * w[0] + (B[2]/A[0]) * w[1]

      # Przesunięcie bufora stanu
      w[1] = w[0]
      w[0] = wn

  return y