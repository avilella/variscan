import javax.swing.*;
import java.awt.event.*;
import java.awt.Container;
import java.awt.*;
import java.awt.print.*;
/**
 * <p>Title: LastWave</p>
 * <p>Description: JNI Interface for LastWave</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: </p>
 * @author John McKelvie
 * @version 1.0
 */

public class java_main extends JFrame {
    private JTextArea terminal;
    private JTextArea keyCodes;
    private JScrollPane scrollKeyCodes, scrollTerminal;
    private JMenuBar menuBar;
    private JMenu fileMenu;
    private String output;
    private String prompt = "Enter Instruction->>";
  public java_main() {
    super("John's test terminal");
    setupGUI();
    affixPrompt();
    resetOutput();
  }
  public void setupGUI()
  { terminal = new JTextArea();
    terminal.addKeyListener( new myKeyListener() );
    terminal.setEditable(false);
    terminal.setLineWrap(true);
    keyCodes = new JTextArea("Keycode:\tCorresponding character\n");
    keyCodes.setEditable(false);
    scrollTerminal = new JScrollPane(terminal);
    scrollTerminal.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
    scrollKeyCodes = new JScrollPane(keyCodes);
    scrollKeyCodes.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED);
    final JDesktopPane theDesktop = new JDesktopPane();
    getContentPane().add( theDesktop );
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
          JOptionPane.ERROR_MESSAGE);
        }
     });

    JMenuItem quit = new JMenuItem("Quit");
    quit.setMnemonic('Q');
    quit.addActionListener( new ActionListener()
    {
        public void actionPerformed( ActionEvent e )
        {
          JOptionPane.showMessageDialog(null, "Thank you for using LastWave", "Goodbye",
          JOptionPane.INFORMATION_MESSAGE);
          System.exit(0);
        }
     });

    fileMenu.add(print);
    fileMenu.add( quit );
    menuBar.add( fileMenu);
    addWindowListener(
			new WindowAdapter()
			{ public void windowClosing( WindowEvent e )
				{ System.exit( 0 ); }
			});
    this.getContentPane().setLayout(null);
    this.getContentPane().add(scrollTerminal);
    scrollTerminal.setBounds(0, 0, 390 , 250);
    this.getContentPane().add(scrollKeyCodes);
    scrollKeyCodes.setBounds(0, 255, 390,150);
    this.setSize( 640, 480 );
    this.setVisible( true );
  }
  public void affixPrompt()
  {
    terminal.append(prompt);
  }
  public void resetOutput()
  { output = "";  }

  public void processSpecialChars(KeyEvent k)
  {
      //    if(specialKeyCode(k).getKeyCode() == KeyEvent.VK_F1)
    if(k.getKeyCode() == KeyEvent.VK_F1)
      System.out.println("F1 Pressed");
     return;
  }
  public void affixChars(char out)
  { // call native method for key processing

    char c = terminalPrintChar(out);
    String addCharToInput = "";

    if( c != '\n' && c != 10 && c != 8)
    { addCharToInput += c;
      output += c;
    }
    // if we want the cursor to go back, do so
    if(c == 8 && terminal.getText().length() > 0  && output.length() > 0)
    { terminal.replaceRange(null, terminal.getText().length() - 1, terminal.getText().length());
      try{
          output = output.substring(0, output.length() -1 );
      }
      catch(StringIndexOutOfBoundsException e)
        {
          e.printStackTrace();
      }
    }
    terminal.append(addCharToInput);
    if(c == '\n' && output.length() > 0)
    { terminal.append( "\n" + output + "\n" );
      resetOutput();
      affixPrompt();
    }

  }

  // New C Implementation of this method
  private native char terminalPrintChar(char c);
  private native void lw_main(String []args);
  private native void terminalPushChar(char c);
  // KeyEvent passed to terminal interpreter
    //  private native KeyEvent specialKeyCode(KeyEvent e);
  private native void XXTerminalPrintChar(char c);

  public void outputKeyCode(char c, int k)
  {
    keyCodes.append("Keycode = " + k);
    if(c == '\n')
      keyCodes.append("\tCorresponding char = " + c );
    else
    keyCodes.append("\tCorresponding char = " + c +"\n");
  }
  public static void main(String[] args) {
    java_main myjava_main = new java_main();
    myjava_main.lw_main(args);
  }
  static{
      try { System.loadLibrary("java_lw"); }
      catch(UnsatisfiedLinkError e){
         System.err.println("LastWave for Java Warning:\n");
         System.err.println("Unable to load library \""+System.mapLibraryName("java_lw")+"\n");
         System.err.println("java.library.path="+System.getProperty("java.library.path","")+"\n");
	 System.err.println(e);
      }
  }
class myKeyListener implements KeyListener{
  public void keyPressed(KeyEvent e)
 {  char c = (char)e.getKeyChar();
    int k = e.getKeyChar();
    terminalPushChar(c);
    processSpecialChars( e );
    affixChars( c );
    outputKeyCode( c , k );
    }
 public void keyReleased(KeyEvent e)
 {}
 public void keyTyped(KeyEvent e)
 {}

}

}
