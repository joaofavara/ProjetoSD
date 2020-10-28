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
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.nio.ByteBuffer;
import java.net.*;

public class Interface extends JFrame{

	TitledBorder title1;
	TitledBorder title2;
	JPanel servidor1;
	JPanel servidor2;
	JLabel dado1;
	JLabel dado2;
	JLabel info1;
	JLabel info2;
	JLabel atividade1;
	JLabel atividade2;
	JLabel time;
	LocalTime lastServer1 = LocalTime.now();
	LocalTime lastServer2 = LocalTime.now();
	
	public Interface() {
		super("Monitoramento");
		setLayout(new FlowLayout(FlowLayout.CENTER));
		
		servidor1 = new JPanel();
		servidor2 = new JPanel();

		dado1 = new JLabel();
		dado2 = new JLabel();

		info1 = new JLabel();
		info2 = new JLabel();

		atividade1 = new JLabel();
		atividade2 = new JLabel();

		time = new JLabel();

		title1 = BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.black), "Servidor 1");
		title2 = BorderFactory.createTitledBorder(BorderFactory.createLineBorder(Color.black), "Servidor 2");

		title1.setTitleJustification(TitledBorder.CENTER);
		title2.setTitleJustification(TitledBorder.CENTER);

		servidor1.setBorder(title1);
		servidor2.setBorder(title2);

		servidor1.setPreferredSize(new Dimension(230,80));
		servidor2.setPreferredSize(new Dimension(230,80));

		servidor1.add(dado1);
		servidor1.add(info1);
		
		servidor2.add(dado2);
		servidor2.add(info2);

		servidor1.add(atividade1);
		servidor2.add(atividade2);

		atividade1.setForeground(Color.green);
		atividade1.setPreferredSize(new Dimension(50,10));
		atividade2.setForeground(Color.red);
		atividade2.setPreferredSize(new Dimension(50,10));

		new Thread(updateTime).start();
		new Thread(updateServer1).start();
		new Thread(updateServer2).start();

		add(servidor1);
		add(servidor2);
		add(time);
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

		if (now.isAfter(lastServer1.plusMinutes(1))) {
			setServidorStatus(1, 2);
		}
		
		if (now.isAfter(lastServer2.plusMinutes(1))) {
			setServidorStatus(2, 2);
		}

		return hours+":"+minutes+":"+seconds;
	}

	private Runnable updateTime = new Runnable() {
		public void run() {

			while (true) {
				time.setText(getTime());
				try { Thread.sleep(1000); } catch (InterruptedException ex) {
					System.out.println ("Puxa, estava dormindo! Você me acordou");
				}
			}
		}
	};

	private Runnable updateServer1 = new Runnable() {
		public void run() {
			try { 
				Socket socket = new Socket();
				InetAddress inetAddress = InetAddress.getByName("localhost");
				SocketAddress socketAddress = new InetSocketAddress(inetAddress, 6000);
				DataInputStream dInp = null;
				int valorRecebido, idCliente, acao;

				while (true) {
					while (!socket.isConnected()) {
						socket.connect(socketAddress);
						if (socket.isConnected()) {
							dInp = new DataInputStream(socket.getInputStream());
							break;
						}
						Thread.sleep(1000);
					}

					if ((dInp != null) && (dInp.available() > 0)) {
						idCliente = dInp.readInt();
						acao = dInp.readInt();
						setInfoServidor (1, idCliente, acao);

						if (acao == 1){
							valorRecebido = dInp.readInt();
							setValorServidor(1, valorRecebido);
						}
						setServidorStatus(1, 1);
						lastServer1 = LocalTime.now();
					}

					Thread.sleep(1000);
				}
			} catch (Exception ex) {
				System.err.print(ex);
			};
		}	
	};

	private Runnable updateServer2 = new Runnable() {
		public void run() {
			try { 
				Socket socket = new Socket();
				InetAddress inetAddress = InetAddress.getByName("localhost");
				SocketAddress socketAddress = new InetSocketAddress(inetAddress, 6001);
				DataInputStream dInp = null;
				int valorRecebido, idCliente, acao;

				while (true) {
					while (!socket.isConnected()) {
						socket.connect(socketAddress);
						if (socket.isConnected()) {
							dInp = new DataInputStream(socket.getInputStream());
							break;
						}
						Thread.sleep(1000);
					}

					if ((dInp != null) && (dInp.available() > 0)) {
						idCliente = dInp.readInt();
						acao = dInp.readInt();
						setInfoServidor (2, idCliente, acao);

						if (acao == 1){
							valorRecebido = dInp.readInt();
							setValorServidor(2, valorRecebido);
						}
						setServidorStatus(2, 1);
						lastServer2 = LocalTime.now();
					}

					Thread.sleep(1000);
				}
			} catch (Exception ex) {
				System.err.print(ex);
			};			
		}
	};

	private void setServidorStatus (int serverId, int serverStatus) {
		if (serverId == 1) {
			if (serverStatus == 1) {
				atividade1.setText("Ativo");
				atividade1.setForeground(Color.green);
			} else if (serverStatus == 2) {
				atividade1.setText("Inativo");
				atividade1.setForeground(Color.red);
			}
		} else if (serverId == 2) {
			if (serverStatus == 1) {
				atividade2.setText("Ativo");
				atividade2.setForeground(Color.green);
			} else if (serverStatus == 2) {
				atividade2.setText("Inativo");
				atividade2.setForeground(Color.red);
			}
		}
	}

	private void setValorServidor (int id, int valor) {
		if (id == 1) {
			dado1.setText("Valor do Dado: "+valor);
		} else if (id == 2) {
			dado2.setText("Valor do Dado: "+valor);
		}	
	}

	private void setInfoServidor (int serverId,int clientId, int acao) {
		if (serverId == 1 && clientId != 0) {
			if (acao == 1) {
				info1.setText("Escrito pelo cliente "+clientId+" - "+getTime());
			} else if (acao == 2) {
				info1.setText("Lido pelo cliente "+clientId+" - "+getTime());
			}
		} else if (serverId == 2 && clientId != 0) {
			if (acao == 1) {
				info2.setText("Escrito pelo cliente "+clientId+" - "+getTime());
			} else if (acao == 2) {
				info2.setText("Lido pelo cliente "+clientId+" - "+getTime());
			}
		}	
	}
}

