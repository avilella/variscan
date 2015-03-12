import javax.swing.*;
import java.awt.*;
import java.awt.event.*;


public class JavaGraphics extends JFrame
{
  public JavaGraphics()
  {
    super("LastWave JavaGraphics Window Alpha Version");
     addWindowListener(
			new WindowAdapter()
			{ public void windowClosing( WindowEvent e )
				{ dispose(); }
			});
    this.setSize(600,480);
    this.setVisible( true );
  }
}
