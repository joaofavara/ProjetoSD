/*
Nome: João Pedro Favara RA: 16061921
Nome: Marcelino Noguero RA: Souza 16011538
Opcionais funcionando: Projeto Basico, Opcionais: 1, 2, 3, 6, 7,8
Observações: Não conseguimos encontrar uma maneira de testar o opcional 6; 
Quando muitos escritores executam juntos, os dados demoram um pouco para replicar os dados.
Valor do Projeto: 
*/

import javax.swing.JFrame;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.Dimension;
import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.BorderFactory;
import javax.swing.border.TitledBorder;
import java.util.Date;
import java.time.LocalTime;
import java.io.*;
import java.nio.ByteBuffer;
import java.net.*;

public class Interface extends JFrame{

	private TitledBorder titulo1;
	private TitledBorder titulo2;
	private JPanel monitorServidor1;
	private JPanel monitorServidor2;
	private JLabel valorDado1;
	private JLabel valorDado2;
	private JLabel acaoInfo1;
	private JLabel acaoInfo2;
	private JLabel statusServidor1;
	private JLabel statusServidor2;
	private JLabel horarioAtual;
	private LocalTime ultimaInteracao1 = LocalTime.now();
	private LocalTime ultimaInteracao2 = LocalTime.now();
	
	public Interface() {
		super("Monitoramento");
		setLayout(new FlowLayout(FlowLayout.CENTER));

		monitorServidor1 = new JPanel();
		titulo1 = BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.black), "Servidor 1");
		titulo1.setTitleJustification(TitledBorder.CENTER);
		monitorServidor1.setBorder(titulo1);
		monitorServidor1.setPreferredSize(new Dimension(230,80));
		valorDado1 = new JLabel();
		valorDado1.setPreferredSize(new Dimension(220,13));
		valorDado1.setHorizontalAlignment(JLabel.CENTER);
		acaoInfo1 = new JLabel();
		acaoInfo1.setPreferredSize(new Dimension(220,13));
		acaoInfo1.setHorizontalAlignment(JLabel.CENTER);
		statusServidor1 = new JLabel();
		statusServidor1.setForeground(Color.green);
		statusServidor1.setPreferredSize(new Dimension(220,13));
		statusServidor1.setHorizontalAlignment(JLabel.CENTER);
		monitorServidor1.add(valorDado1);
		monitorServidor1.add(acaoInfo1);
		monitorServidor1.add(statusServidor1);
		
		titulo2 = BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.black), "Servidor 2");
		titulo2.setTitleJustification(TitledBorder.CENTER);
		monitorServidor2 = new JPanel();
		monitorServidor2.setBorder(titulo2);
		monitorServidor2.setPreferredSize(new Dimension(230,80));
		valorDado2 = new JLabel();
		valorDado2.setPreferredSize(new Dimension(220,13));
		valorDado2.setHorizontalAlignment(JLabel.CENTER);
		acaoInfo2 = new JLabel();
		acaoInfo2.setPreferredSize(new Dimension(220,13));
		acaoInfo2.setHorizontalAlignment(JLabel.CENTER);
		statusServidor2 = new JLabel();
		statusServidor2.setForeground(Color.red);
		statusServidor2.setPreferredSize(new Dimension(220,13));
		statusServidor2.setHorizontalAlignment(JLabel.CENTER);
		monitorServidor2.add(valorDado2);
		monitorServidor2.add(acaoInfo2);	
		monitorServidor2.add(statusServidor2);

		horarioAtual = new JLabel();

		new Thread(updateTime).start();
		new Thread(updateServer1).start();
		new Thread(updateServer2).start();

		add(monitorServidor1);
		add(monitorServidor2);
		add(horarioAtual);
	}

	private String getTime() {
		LocalTime now = LocalTime.now();
		String hours = Integer.toString(now.getHour());
		String minutes = Integer.toString(now.getMinute());
		String seconds = Integer.toString(now.getSecond());

		if (Integer.valueOf(hours) < 10) {
			hours = "0"+hours;
		}

		if (Integer.valueOf(minutes) < 10) {
			minutes = "0"+minutes;
		}

		if (Integer.valueOf(seconds) < 10) {
			seconds = "0"+seconds;
		}

		if (!statusServidor1.getText().equals("Offline") && now.isAfter(ultimaInteracao1.plusMinutes(1))) {
			setServidorStatus(1, 2);
		}
		
		if (!statusServidor2.getText().equals("Offline") && now.isAfter(ultimaInteracao2.plusMinutes(1))) {
			setServidorStatus(2, 2);
		}

		return hours+":"+minutes+":"+seconds;
	}

	private Runnable updateTime = new Runnable() {
		public void run() {

			while (true) {
				horarioAtual.setText(getTime());
				try { Thread.sleep(1000); } catch (InterruptedException ex) {}
			}
		}
	};

	private Runnable updateServer1 = new Runnable() {
		public void run() {

			try { 
				Socket socket = new Socket();
				InetAddress inetAddress = InetAddress.getByName("localhost");
				DataInputStream dInp = null;
				int valorRecebido, idCliente, acao;

				while (true) {
					while (true) {
						try {
							if (socket.isClosed()){
								socket = new Socket();
							}
							if (!socket.isConnected()) {
								socket.connect(new InetSocketAddress(inetAddress, 6000));
								if (socket.isConnected()) {
									setServidorStatus(1, 1);
									dInp = new DataInputStream(socket.getInputStream());
									break;
								}
							} else {
								break;
							}
							
						} catch (SocketException ex) {
							setServidorStatus(1, 3);
							socket.close();
						}
					}

					try {
						idCliente = dInp.readInt();
						acao = dInp.readInt();
						setInfoServidor (1, idCliente, acao);

						if (acao == 1){
							valorRecebido = dInp.readInt();
							setValorServidor(1, valorRecebido);
						}
						setServidorStatus(1, 1);
					} catch (EOFException ex) {
						socket.close();
						continue;
					}

					Thread.sleep(1000);
				}
			} catch (InterruptedException | IOException ex) {
				System.out.println(ex.toString());
				setServidorStatus(1, 3);
			};		
		}	
	};

	private Runnable updateServer2 = new Runnable() {
		public void run() {

			try { 
				Socket socket = new Socket();
				InetAddress inetAddress = InetAddress.getByName("localhost");
				DataInputStream dInp = null;
				int valorRecebido, idCliente, acao;

				while (true) {
					while (true) {
						try {
							if (socket.isClosed()) {
								socket = new Socket();
							}
							if (!socket.isConnected()) {
								socket.connect(new InetSocketAddress(inetAddress, 6001));
								if (socket.isConnected()) {
									setServidorStatus(2, 1);
									dInp = new DataInputStream(socket.getInputStream());
									break;
								}
							} else {
								break;
							}

						} catch (SocketException ex) {
							setServidorStatus(2, 3);
							socket.close();
						}	
					}
					
					try {
						idCliente = dInp.readInt();
						acao = dInp.readInt();
						setInfoServidor (2, idCliente, acao);

						if (acao == 1){
							valorRecebido = dInp.readInt();
							setValorServidor(2, valorRecebido);
						}
						setServidorStatus(2, 1);
					} catch (EOFException ex) {
						socket.close();
						continue;
					}

					Thread.sleep(1000);
				}
			} catch (InterruptedException | IOException ex) {
				System.err.println(ex.toString());
				setServidorStatus(2, 3);
			};		
		}
	};

	private void setServidorStatus (int idServidor, int statusServidor) {
		if (idServidor == 1) {
			if (statusServidor == 1) {
				ultimaInteracao1 = LocalTime.now();
				statusServidor1.setText("Ativo");
				statusServidor1.setForeground(Color.green);
			} else if (statusServidor == 2) {
				statusServidor1.setText("Inativo");
				statusServidor1.setForeground(Color.red);
			} else if (statusServidor == 3) {
				statusServidor1.setText("Offline");
				statusServidor1.setForeground(Color.red);
				try { Thread.sleep(1000); } catch (InterruptedException ex) {}
			}
		} else if (idServidor == 2) {
			if (statusServidor == 1) {
				ultimaInteracao2 = LocalTime.now();
				statusServidor2.setText("Ativo");
				statusServidor2.setForeground(Color.green);
			} else if (statusServidor == 2) {
				statusServidor2.setText("Inativo");
				statusServidor2.setForeground(Color.red);
			} else if (statusServidor == 3) {
				statusServidor2.setText("Offline");
				statusServidor2.setForeground(Color.red);
				try { Thread.sleep(1000); } catch (InterruptedException ex) {}
			}
		}
	}

	private void setValorServidor (int idServidor, int valor) {
		if (idServidor == 1) {
			valorDado1.setText("Valor do Dado: "+valor);
		} else if (idServidor == 2) {
			valorDado2.setText("Valor do Dado: "+valor);
		}	
	}

	private void setInfoServidor (int idServidor,int idCliente, int acao) {
		if (idServidor == 1 && idCliente != 0) {
			if (acao == 1) {
				acaoInfo1.setText("Escrito pelo cliente "+idCliente+" - "+getTime());
			} else if (acao == 2) {
				acaoInfo1.setText("Lido pelo cliente "+idCliente+" - "+getTime());
			}
		} else if (idServidor == 2 && idCliente != 0) {
			if (acao == 1) {
				acaoInfo2.setText("Escrito pelo cliente "+idCliente+" - "+getTime());
			} else if (acao == 2) {
				acaoInfo2.setText("Lido pelo cliente "+idCliente+" - "+getTime());
			}
		}	
	}
}

