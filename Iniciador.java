/*
Nome: João Pedro Favara RA: 16061921
Nome: Marcelino Noguero RA: Souza 16011538
Opcionais funcionando: Projeto Basico, Opcionais: 1, 2, 3, 6, 7,8
Observações: Não conseguimos encontrar uma maneira de testar o opcional 6; 
Quando muitos escritores executam juntos, os dados demoram um pouco para replicar os dados.
Valor do Projeto: 
*/

import javax.swing.JFrame;

public class Iniciador {
	
	public static void main (String[] args)
	{
		Interface menuMonitoramento = new Interface();
		menuMonitoramento.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		menuMonitoramento.setSize(500,150);
		menuMonitoramento.setResizable(false);
		menuMonitoramento.setLocationRelativeTo(null);
		menuMonitoramento.setVisible(true);
	}
}