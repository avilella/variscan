/*	This listing is the entry point for LastWave's Java Interface...
**	Java instantiates the JavaTerminal class
**	which first loads the LastWave shared library
**	then calls the main() function in int_main.c in LastWave.
**	This class manages terminal I/O -
**	Input - deals with KeyEvents and sends Java chars / KeyCodes to LastWave through
**	the sendKeyCode() native method implemented in java_terminal.c .
**	Output - LastWave :
	 prints strings of characters
	 prints individual characters
	 moves cursor
	 deletes characters using the XXTerminal** functions implemented in java_terminal.c .

*/
import java.awt.*;
import java.awt.event.*;
import java.awt.event.KeyEvent;
import java.awt.Toolkit.*;
import javax.swing.*;
import javax.swing.text.*;
import javax.swing.event.*;

public class JavaTerminal extends JFrame{

    private JTextArea terminal;
    private JTextArea keyCodes;
    private JScrollPane scrollKeyCodes, scrollTerminal;
    private JMenuBar menuBar;
    private JMenu fileMenu;

    // Constructor method for this class calls JFrame superclass constructor
    // to affix title to terminal window
    public JavaTerminal()
    {	super("LastWave Java Terminal - Alpha Version!!!");

    }

    public void setupTerminalGUI()
    {
      // Initialise all GUI in 'this' JFrame
      // Each element is a lightweight Swing component...
      // see java.sun.com documentation for details.
    terminal = new JTextArea();
    // Listerner defined in inner class myKeyListener
    terminal.addKeyListener( new myKeyListener() );
    terminal.setEditable(false);
    terminal.setLineWrap(true);


    keyCodes = new JTextArea("Keycode:\tCorresponding character\n");
    keyCodes.setEditable(false);
    scrollTerminal = new JScrollPane(terminal);
    scrollTerminal.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
    scrollKeyCodes = new JScrollPane(keyCodes);
    scrollKeyCodes.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);

    menuBar = new JMenuBar();
    setJMenuBar(menuBar);
    fileMenu = new JMenu("File");
    fileMenu.setMnemonic('F');
    JMenuItem print = new JMenuItem("Print");
    print.setMnemonic('P');
    print.addActionListener( new ActionListener()
    {
        public void actionPerformed( ActionEvent e )
        {
          JOptionPane.showMessageDialog(null, "This function has not yet been implemented", "Print",
          JOptionPane.INFORMATION_MESSAGE);
        }
     });

    JMenuItem quit = new JMenuItem("Quit");
    quit.setMnemonic('Q');
    quit.addActionListener( new ActionListener()
    {
        public void actionPerformed( ActionEvent e )
        {
	    int i = JOptionPane.showConfirmDialog(null, "Are you sure you want to close the terminal ?\nDoing so will exit LastWave\n", "Exit ?",
	    JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE,null);
	    if(i == JOptionPane.NO_OPTION)
	    return;
	    else
          System.exit(0);
        }
     });

    fileMenu.add( print );
    fileMenu.add( quit );
    menuBar.add( fileMenu);
    addWindowListener(
			new WindowAdapter()
			{ public void windowClosing( WindowEvent e )
				{ System.exit( 0 ); }
			});
    this.getContentPane().setLayout(null);
    this.getContentPane().add(scrollTerminal);
    scrollTerminal.setBounds(0, 0, 790 ,400);
    this.getContentPane().add(scrollKeyCodes);
    scrollKeyCodes.setBounds(0,420, 790,250);
    this.setSize( 800, 600 );
    this.setVisible( true );

	}



	public void printChar(char c)
	{
	  // Just print a character
	  String out = "";
	  out += c;
	  terminal.append(out);
	}
        public void deleteChar()
        {  String str = " ";
	//terminal.insert(str, terminal.getText().length()-1);
	terminal.replaceRange(null, terminal.getText().length()-1,terminal.getText().length());
	}
	public void printString(String myString)
        { // Print a String - called from XXTerminalPrintStr
          // and XXTerminalPrintErrStr in Lastwave(java_terminal.c)
		terminal.append(myString);
		//make sure the terminal always scrolls to the bottom of the terminal window
		terminal.scrollRectToVisible(new Rectangle(0,terminal.getHeight()-2,1,1));
		terminal.setCaretPosition(terminal.getText().length());
	}
        public void beep()
        {
	  Toolkit.getDefaultToolkit().beep();
        }


	public static void main(String args[])
	{
		System.out.println("Java main : start");
             	JavaTerminal lwTerminal = new JavaTerminal();

		System.out.println("Java main : new JavaTerminal created");
		// initialise Java Terminal GUI  and add JComponents (e.g. JTextAreas / JMenus) to JFrame
		lwTerminal.setupTerminalGUI();
		System.out.println("Java main : GUI has been setup - trying to init JNI");
		// initialise native JNIEnv and jobject environment
		lwTerminal.initJNITerminal();
		System.out.println("Java main : JNI Terminal has been initialized");

		lwTerminal.lw_main( args );
		System.out.println("Java main : C main has been started");

		System.exit(0);

	}

	private native void lw_main(String [] args);
	// This method is only called one time
	private native void initJNITerminal();
        private native void sendKeyChar(char c,int flagUp);
        private native void sendKeyCode(int keyCode,int flagUp);


	static
	{
		System.loadLibrary("lw");
		System.out.println("Java : library lw has been loaded");
	}

	class myKeyListener implements KeyListener{
		public void keyPressed(KeyEvent e)
	  {
	    char c = (char)e.getKeyChar();
	    int  k = e.getKeyCode();

	    sendKeyChar(c,0);

	    sendKeyCode(k,0);

	  }
 	public void keyReleased(KeyEvent e)
	{ /*
	  char c = (char)e.getKeyChar();
	  int  k = e.getKeyCode();
	  sendKeyChar(c,1);
	  sendKeyCode(k,1);
	  */
	}
 	public void keyTyped(KeyEvent e)
	  {
	  }

	}
}
