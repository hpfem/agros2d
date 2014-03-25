from pairparticle_tracing import load_data, plot_data

if __name__ == '__main__':
  data = load_data('free_fall')
  data_a2d = load_data('free_fall-a2d')
  plot_data(data + data_a2d, 'comparision')
