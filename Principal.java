import javax.swing.JFrame;

public class Principal {
	
	public static void main (String[] args)
	{
		Interface menuPrincipal = new Interface();
		menuPrincipal.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		menuPrincipal.setSize(500,150);
		menuPrincipal.setResizable(false);
		menuPrincipal.setLocationRelativeTo(null);
		menuPrincipal.setVisible(true);
	}
}