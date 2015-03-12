import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.text.*;
import javax.swing.event.*;

public class VariScanGUI implements ActionListener, ItemListener {
	String shell;
	String shellParam;
	String perlBin;
	
	JFrame vsConfigEditFrame, lwConfigEditFrame; 
	
	JFileChooser fc;
	JButton vsAliButton, vsConfigButton, vsOutputButton,
		vsOnlyRunButton, vsConfigEditButton, vsConfigCreateButton, 
		lwInputButton, lwConfigButton, lwConfigCreateButton, lwConfigEditButton,
		lwOutputButton, lwOnlyRunButton, analysisRunButton;
	JTextField vsAliText, vsConfigText, vsOutputText, lwInputText, lwConfigText, 
		lwOutputText;
	JLabel statusLabel;
	JProgressBar progress;
	
	
	File vsBinFile = new File("");
	File lwDir = new File(""); 
	File scriptDir = new File(""); 
	File vsAliFile = new File(""); 
	File vsConfigFile = new File("");
	File vsBdfFile = new File("");
	File vsOutputFile = new File(""); 
	File lwBinFile = new File(""); 
	File lwConfigFile = new File(""); 
	File lwSourceDir = new File(""); 
	File lwInputFile = new File(""); 
	File lwOutputFile = new File(""); 
	File visInputFile = new File(""); 
	File visOutputFile = new File("");
	
	
	File settingsFile = new File("VSsettings.ini");
	File recentFile = new File("VSrecent.ini");
	String line;
	GridBagConstraints c = new GridBagConstraints();

	
	//components of the content pane
	JPanel mainPanel, vsPanel, lwPanel, visPanel;
	JCheckBox vsEnableCheck, lwEnableCheck, visEnableCheck;
	
	//components of the settings panel
	JButton settingsVsBinButton, settingsLwBinButton, settingsScriptDirButton, 
		settingsSaveButton, settingsLwDirButton, settingsLwSourceDirButton;
	JTextField settingsVsBinText, settingsLwBinText, settingsScriptDirText, 
		settingsLwDirText, settingsLwSourceDirText;
	
	//variscan config File parameters
	String startPos, endPos, refPos, bdfFile, indivNames, seqChoice, outgroup,
		refSeq, runMode, useMuts, completeDeletion, fixNum, numNuc, 
		slidingWindow, widthSW, jumpSW, windowType, useLdSinglets;
			
	//the components of the variscan config editor;
	JTextField editStartPosText, editEndPosText, editBdfFileText, editIndivNamesText,
		editSeqChoiceText, editOutgroupText, editRefSeqText, editNumNucText, 
		editWidthSwText, editJumpSwText;
	JComboBox editRefPosBox, editSeqChoiceBox, editOutgroupBox, editRunModeBox, 
		editWindowTypeBox;
	JCheckBox editBdfFileCheck, editUseMutsCheck, editUseSingletsCheck, 
		editCompleteDeletionCheck, editFixNumCheck, editSlidingWindowCheck;
	JButton editBdfFileButton, editSaveButton, editCancelButton;
	JLabel editBdfFileTextLabel, editSeqChoiceTextLabel, editOutgroupTextLabel,
		editFixNumLabel, editNumNucLabel, editWidthSwLabel, editJumpSwLabel, 
		editWindowTypeLabel;
	
	
	// lastwave config file parameters;
	String lwStatCol, lwRefCol, lwSubA, lwSubB, lwSubC, lwFilter;
	
	//the components of the lastwave config editor
	JTextField lwStatColText, lwRefColText, lwSubAText, lwSubBText, lwSubCText, 
		lwFilterText;
	JButton lwCancelButton, lwSaveButton;
	
	// components for visualization
	JTextField visInputText, visOutputText, visChromIdText;
	JButton visInputButton, visOutputButton;
	JComboBox visTypeBox;
	
	int visType = 0;


    public JMenuBar createMenuBar() {
        JMenuBar menuBar;
        JMenu tasksMenu, optionsMenu, helpMenu;
        JMenuItem vsItem, lwItem, allItem, settingsItem, aboutItem;

        //Create the menu bar.
        menuBar = new JMenuBar();

		//Menus for menu bar
        tasksMenu = new JMenu("Tasks");
        optionsMenu = new JMenu("Options");
        helpMenu = new JMenu("Help");
        menuBar.add(tasksMenu);
        menuBar.add(optionsMenu);
        menuBar.add(helpMenu);
        
        //JMenuItems for Tasks
        allItem = new JMenuItem("Run Analysis");
        allItem.getAccessibleContext().setAccessibleName("Analysis");
        allItem.addActionListener(this);
        tasksMenu.add(allItem);
        
        
        
        //JMenuItems for Options
        settingsItem = new JMenuItem("Settings");
        settingsItem.getAccessibleContext().setAccessibleName("Settings");
        settingsItem.addActionListener(this);
        optionsMenu.add(settingsItem);
        
        //JMenuItems for Help
        aboutItem = new JMenuItem("About");
        aboutItem.getAccessibleContext().setAccessibleName("About");
        aboutItem.addActionListener(this);
        helpMenu.add(aboutItem);
        
      
        return menuBar;
    }

    public JPanel createMainPanel() {
        
        //Create the content pane
        mainPanel = new JPanel(new CardLayout());
       
        //set global features for GridBagLayout
		c.insets = new Insets(5,5,5,5);
		c.weightx = c.weighty = 1;

		// Create the "about" panel
		JPanel aboutNamePanel = new JPanel();
		aboutNamePanel.setLayout(new BoxLayout(aboutNamePanel, BoxLayout.Y_AXIS));
		JLabel aboutText = new JLabel("GUI for VariScan");
		aboutText.setFont(new Font("Serif",Font.PLAIN,24));
		aboutText.setAlignmentX(Component.CENTER_ALIGNMENT);
		JLabel aboutVersion = new JLabel("Version 0.2beta");
		aboutVersion.setFont(new Font("Serif",Font.PLAIN,24));
		aboutVersion.setAlignmentX(Component.CENTER_ALIGNMENT);
		
		JTextArea helpText = new JTextArea
			("The VariScan GUI is a graphical user interface\n" +
			"to help run VariScan and associated programs.\n\n" +
			"It was written by Stephan Hutter and published\n" +
			"under the GNU General Public License.\n\n" +
			"For the latest files and documentation please go to:\n" +
			"http://www.ub.es/softevol/variscan"); 
		helpText.setEditable(false);
		helpText.setMaximumSize(helpText.getPreferredSize());

		aboutNamePanel.add(Box.createVerticalGlue());
		aboutNamePanel.add(aboutText);
		aboutNamePanel.add(aboutVersion);
		aboutNamePanel.add(Box.createVerticalGlue());
		aboutNamePanel.add(helpText);
		aboutNamePanel.add(Box.createVerticalGlue());
		
		JPanel aboutPanel = new JPanel();
		aboutPanel.setLayout(new BoxLayout(aboutPanel, BoxLayout.PAGE_AXIS));
		aboutNamePanel.setAlignmentX(Component.CENTER_ALIGNMENT);
		aboutPanel.add(aboutNamePanel);
		
		// Create the "settings" panel
		JPanel settingsPanel = new JPanel();
		settingsPanel.setLayout(new BoxLayout(settingsPanel, BoxLayout.PAGE_AXIS));
		
		JPanel settingsSavePanel = new JPanel();
		settingsSaveButton = new JButton("Save Settings");
		settingsSaveButton.addActionListener(this);
		settingsSavePanel.add(settingsSaveButton);
		
		JPanel settingsUpperPanel = new JPanel();
		settingsUpperPanel = createSettingsPanel();
				
		settingsPanel.add(settingsUpperPanel);
		settingsPanel.add(settingsSavePanel);
		
		// Create the "analysis" panel
		JPanel analysisPanel = new JPanel();
		analysisPanel.setLayout(new BoxLayout(analysisPanel, BoxLayout.PAGE_AXIS));
		
		//RUN button
		JPanel analysisRunPanel = new JPanel();
		analysisRunButton = new JButton("RUN!");
		analysisRunButton.addActionListener(this);
		analysisRunPanel.add(analysisRunButton);
		

		//create the checkboxes
		JPanel vsEnablePanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		vsEnableCheck = new JCheckBox("Perform VariScan analysis", true);
		vsEnableCheck.addItemListener(this);
		vsEnablePanel.add(vsEnableCheck);
		
		JPanel lwEnablePanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		lwEnableCheck = new JCheckBox("Perform Lastwave analysis", true);
		lwEnableCheck.addItemListener(this);
		lwEnablePanel.add(lwEnableCheck);

		JPanel visEnablePanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
		visEnableCheck = new JCheckBox("Generate visualization", true);
		visEnableCheck.addItemListener(this);
		visEnablePanel.add(visEnableCheck);
		
		vsPanel = new JPanel();
		vsPanel = createVsPanel();
		lwPanel = new JPanel();
		lwPanel = createLwPanel();
		visPanel = new JPanel();
		visPanel = createVisPanel();
		
		analysisPanel.add(vsEnablePanel);
		analysisPanel.add(vsPanel);
		analysisPanel.add(lwEnablePanel);
		analysisPanel.add(lwPanel);
		analysisPanel.add(visEnablePanel);
		analysisPanel.add(visPanel);
		analysisPanel.add(analysisRunPanel);
		
				
        //Assemble the cards in the content pane
        mainPanel.add(analysisPanel, "Analysis");
        mainPanel.add(aboutPanel, "About");
        mainPanel.add(settingsPanel, "Settings");

        //initialize the file chooser
        fc = new JFileChooser();
        
        return mainPanel;
    }
    
    public JPanel createStatusPanel () {
		//create the progress panel
		JPanel statusPanel = new JPanel(new GridBagLayout());
		statusLabel = new JLabel("Status: Ready");
		progress = new JProgressBar();
		
		c.gridx = c.gridy = 0;
		c.anchor = GridBagConstraints.LINE_START;
		statusPanel.add(statusLabel, c);
		c.gridx = 1;
		c.anchor = GridBagConstraints.LINE_END;
		statusPanel.add(progress, c);
		
		statusPanel.setBorder(BorderFactory.createLoweredBevelBorder());
		return statusPanel;
    }
    
    public JPanel createVsPanel (){
    	
		JPanel vsPanel = new JPanel(new GridBagLayout());
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;

		
		JLabel vsAliLabel = new JLabel("Sequence alignment file:");
		c.gridx = c.gridy = 0;
		vsPanel.add(vsAliLabel, c);
		vsAliText = new JTextField(30);
		vsAliText.setEditable(false);
		c.gridx++;
		vsPanel.add(vsAliText, c);
		vsAliButton = new JButton("Browse...");
		vsAliButton.addActionListener(this);
		c.gridx++;
		vsPanel.add(vsAliButton, c);
		
		JLabel vsConfigLabel = new JLabel("VariScan config file:");
		c.gridy++;
		c.gridx = 0;
		vsPanel.add(vsConfigLabel, c);
		vsConfigText = new JTextField(30);
		vsConfigText.setEditable(false);
		c.gridx++;
		vsPanel.add(vsConfigText, c);
		vsConfigButton = new JButton("Browse...");
		vsConfigButton.addActionListener(this);
		c.gridx++;
		vsPanel.add(vsConfigButton, c);
		
		
		vsConfigEditButton = new JButton("Edit...");
		c.gridy++;
		c.gridx = 1;
		c.anchor = GridBagConstraints.EAST;
		c.fill = GridBagConstraints.NONE;
		vsConfigEditButton.addActionListener(this);
		vsPanel.add(vsConfigEditButton, c);
		c.fill = GridBagConstraints.HORIZONTAL;
		vsConfigCreateButton = new JButton("Create...");
		c.gridx++;
		vsConfigCreateButton.addActionListener(this);
		vsPanel.add(vsConfigCreateButton, c);
		
		JLabel vsOutputLabel = new JLabel("VariScan output file:");
		c.anchor = GridBagConstraints.WEST;
		c.gridy++;
		c.gridx = 0;
		vsPanel.add(vsOutputLabel, c);
		vsOutputText = new JTextField(30);
		vsOutputText.setEditable(false);
		c.gridx++;
		vsPanel.add(vsOutputText, c);
		vsOutputButton = new JButton("Browse...");
		vsOutputButton.addActionListener(this);
		c.gridx++;
		vsPanel.add(vsOutputButton, c);
		
		TitledBorder border;
		border = BorderFactory.createTitledBorder("VariScan Parameters");
		border.setTitleJustification(TitledBorder.CENTER);
		vsPanel.setBorder(border);
		
		return vsPanel;    
	}
	
	public JPanel createLwPanel () {
		JPanel lwPanel = new JPanel(new GridBagLayout());
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;

		JLabel lwInputLabel = new JLabel("LastWave input file:");
		c.gridx = c.gridy = 0;
		lwPanel.add(lwInputLabel, c);
		lwInputText = new JTextField(30);
		lwInputText.setEditable(false);
		c.gridx++;
		lwPanel.add(lwInputText, c);
		lwInputButton = new JButton("Browse...");
		lwInputButton.addActionListener(this);
		c.gridx++;
		lwPanel.add(lwInputButton, c);
		
		JLabel lwConfigLabel = new JLabel("LastWave config file:");
		c.gridy++;
		c.gridx = 0;
		lwPanel.add(lwConfigLabel, c);
		lwConfigText = new JTextField(30);
		lwConfigText.setEditable(false);
		c.gridx++;
		lwPanel.add(lwConfigText, c);
		lwConfigButton = new JButton("Browse...");
		lwConfigButton.addActionListener(this);
		c.gridx++;
		lwPanel.add(lwConfigButton, c);
		
		lwConfigEditButton = new JButton("Edit...");
		c.gridy++;
		c.gridx = 1;
		c.anchor = GridBagConstraints.EAST;
		c.fill = GridBagConstraints.NONE;
		lwConfigEditButton.addActionListener(this);
		lwPanel.add(lwConfigEditButton, c);
		c.fill = GridBagConstraints.HORIZONTAL;
		lwConfigCreateButton = new JButton("Create...");
		c.gridx++;
		lwConfigCreateButton.addActionListener(this);
		lwPanel.add(lwConfigCreateButton, c);
		
		JLabel lwOutputLabel = new JLabel("LastWave output file:");
		c.anchor = GridBagConstraints.WEST;
		c.gridy++;
		c.gridx = 0;
		lwPanel.add(lwOutputLabel, c);
		lwOutputText = new JTextField(30);
		lwOutputText.setEditable(false);
		c.gridx++;
		lwPanel.add(lwOutputText, c);
		lwOutputButton = new JButton("Browse...");
		lwOutputButton.addActionListener(this);
		lwOutputButton.setEnabled(false);
		c.gridx++;
		lwPanel.add(lwOutputButton, c);

		TitledBorder border;
		border = BorderFactory.createTitledBorder("LastWave Parameters");
		border.setTitleJustification(TitledBorder.CENTER);
		lwPanel.setBorder(border);

		return lwPanel;    
	}
	
	public JPanel createVisPanel () {
		JPanel panel = new JPanel(new GridBagLayout());
		c.fill = GridBagConstraints.HORIZONTAL;
		
		JLabel visInputLabel = new JLabel("Visualization input file:");
		c.gridx = c.gridy = 0;
		panel.add(visInputLabel, c);
		visInputText = new JTextField(30);
		visInputText.setEditable(false);
		c.gridx++;
		panel.add(visInputText, c);
		visInputButton = new JButton("Browse...");
		visInputButton.addActionListener(this);
		c.gridx++;
		panel.add(visInputButton, c);

		JLabel visOutputLabel = new JLabel("Visualization Output file:");
		c.gridx = 0;
		c.gridy++;
		panel.add(visOutputLabel, c);
		visOutputText = new JTextField(30);
		visOutputText.setEditable(false);
		c.gridx++;
		panel.add(visOutputText, c);
		visOutputButton = new JButton("Browse...");
		visOutputButton.addActionListener(this);
		visOutputButton.setEnabled(false);
		c.gridx++;
		panel.add(visOutputButton, c);
		
		JLabel visChromIdLabel = new JLabel("Chromosome ID:");
		c.gridx = 0;
		c.gridy++;
		panel.add(visChromIdLabel, c);
		visChromIdText = new JTextField(2);
		c.gridx++;
		c.fill = GridBagConstraints.NONE;
		panel.add(visChromIdText, c);
		
		JLabel visTypeLabel = new JLabel("Visualization type:");
		c.gridx = 0;
		c.gridy++;
		panel.add(visTypeLabel, c);
		String visTypeStrings[] = {"UCSC wiggle bed", "GBrowse intensity xyplot"};
		visTypeBox = new JComboBox (visTypeStrings);
		visTypeBox.addItemListener(this);
		c.gridx++;
		panel.add(visTypeBox, c);
		
		TitledBorder border;
		border = BorderFactory.createTitledBorder("Visualization Parameters");
		border.setTitleJustification(TitledBorder.CENTER);
		panel.setBorder(border);
		
		return panel;

	}
	
	
	public JPanel createSettingsPanel () {
		JPanel panel = new JPanel(new GridBagLayout());
		c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.HORIZONTAL;
		
		c.gridx = c.gridy = 0;
		JLabel settingsScriptDirLabel = new JLabel("Scripts directory:");
		panel.add(settingsScriptDirLabel, c);
		settingsScriptDirText = new JTextField(30);
		settingsScriptDirText.setMinimumSize(settingsScriptDirText.getPreferredSize());
		settingsScriptDirText.setEditable(false);
		c.gridx++;
		panel.add(settingsScriptDirText, c);
		settingsScriptDirButton = new JButton("Browse...");
		settingsScriptDirButton.addActionListener(this);
		c.gridx++;
		panel.add(settingsScriptDirButton, c);

		c.gridx = 0;
		c.gridy++;
		JLabel settingsVsBinLabel = new JLabel("VariScan binary:");
		panel.add(settingsVsBinLabel, c);
		settingsVsBinText = new JTextField(30);
		settingsVsBinText.setEditable(false);
		c.gridx++;
		panel.add(settingsVsBinText, c);
		settingsVsBinButton = new JButton("Browse...");
		settingsVsBinButton.addActionListener(this);
		c.gridx++;
		panel.add(settingsVsBinButton, c);
		
		c.gridy++;
		c.gridx = 0;
		JLabel settingsLwDirLabel = new JLabel("LastWave directory:");
		panel.add(settingsLwDirLabel, c);
		settingsLwDirText = new JTextField(30);
		settingsLwDirText.setEditable(false);
		c.gridx++;
		panel.add(settingsLwDirText, c);
		settingsLwDirButton = new JButton("Browse...");
		settingsLwDirButton.addActionListener(this);
		c.gridx++;
		panel.add(settingsLwDirButton, c);
		
		c.gridy++;
		c.gridx = 0;
		JLabel settingsLwSourceDirLabel = new JLabel("LastWave source directory:");
		panel.add(settingsLwSourceDirLabel, c);
		settingsLwSourceDirText = new JTextField(30);
		settingsLwSourceDirText.setEditable(false);
		c.gridx++;
		panel.add(settingsLwSourceDirText, c);
		settingsLwSourceDirButton = new JButton("Browse...");
		settingsLwSourceDirButton.addActionListener(this);
		c.gridx++;
		panel.add(settingsLwSourceDirButton, c);
		
		c.gridy++;
		c.gridx = 0;
		JLabel settingsLwBinLabel = new JLabel("LastWave binary:");
		panel.add(settingsLwBinLabel, c);
		settingsLwBinText = new JTextField(30);
		settingsLwBinText.setEditable(false);
		c.gridx++;
		panel.add(settingsLwBinText, c);
		settingsLwBinButton = new JButton("Browse...");
		settingsLwBinButton.addActionListener(this);
		c.gridx++;
		panel.add(settingsLwBinButton, c);
		
		panel.setMaximumSize(panel.getPreferredSize());
		
		return panel;
	}
	
    public Container createVsConfigEditPanel () {
    	// Initialize everything in a way that all fields will be enabled
    	
    	JPanel parameterPanel = new JPanel(new GridBagLayout());
    	c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
    	
    	JLabel editStartPosLabel = new JLabel("Start position:");
    	c.gridx = c.gridy = 0;
    	parameterPanel.add(editStartPosLabel, c);
    	editStartPosText = new JTextField(12);
    	editStartPosText.addActionListener(this);
		c.gridx = 1;
    	parameterPanel.add(editStartPosText, c);

    	JLabel editEndPosLabel = new JLabel("End position:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(editEndPosLabel, c);
    	editEndPosText = new JTextField(12);
		editEndPosText.addActionListener(this);
		c.gridx++;
    	parameterPanel.add(editEndPosText, c);

    	JLabel editRefPosLabel = new JLabel("Positions relative to:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(editRefPosLabel, c);
    	String refPosStrings[] = {"Alignment", "Reference sequence"};
    	editRefPosBox = new JComboBox(refPosStrings);
    	editRefPosBox.addItemListener(this);
		c.gridx++;
    	parameterPanel.add(editRefPosBox, c);

		JLabel editRefSeqLabel = new JLabel("Reference sequence:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(editRefSeqLabel, c);
    	editRefSeqText = new JTextField(2);
    	editRefSeqText.addActionListener(this);
    	c.gridx++;
    	parameterPanel.add(editRefSeqText, c);

    	JLabel editBdfFileLabel = new JLabel("Use BDF file?");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(editBdfFileLabel, c);
    	c.gridx++;
    	editBdfFileCheck = new JCheckBox("",true);
    	editBdfFileCheck.addItemListener(this);
    	parameterPanel.add(editBdfFileCheck, c);
	   	
	   	editBdfFileTextLabel = new JLabel("        Location of BDF file:");
	   	c.gridy++;
    	c.gridx = 0;
    	parameterPanel.add(editBdfFileTextLabel, c);
    	editBdfFileText = new JTextField(30);
    	editBdfFileText.setEditable(false);
    	editBdfFileText.addActionListener(this);
	   	c.gridx++;
	   	parameterPanel.add(editBdfFileText, c);
    	editBdfFileButton = new JButton("Browse...");
		editBdfFileButton.addActionListener(this);
		c.gridx++;
		parameterPanel.add(editBdfFileButton, c);
		
		JLabel editIndivNamesLabel = new JLabel("Individual Names:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editIndivNamesLabel, c);
		c.gridx++;
		editIndivNamesText = new JTextField(30);
		editIndivNamesText.addActionListener(this);
		parameterPanel.add(editIndivNamesText, c);
		
		JLabel editSeqChoiceLabel = new JLabel("Included Sequences:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editSeqChoiceLabel, c);
		c.gridx++;
		String seqChoiceStrings[] = {"All", "Custom vector"};
		editSeqChoiceBox = new JComboBox(seqChoiceStrings);
		editSeqChoiceBox.setSelectedIndex(1);
		editSeqChoiceBox.addItemListener(this);
		parameterPanel.add(editSeqChoiceBox, c);
		
	   	editSeqChoiceTextLabel = new JLabel("        Custom vector:");
	   	c.gridy++;
    	c.gridx = 0;
    	parameterPanel.add(editSeqChoiceTextLabel, c);
		editSeqChoiceText = new JTextField(30);
		editSeqChoiceText.addActionListener(this);
		c.gridx++;
		parameterPanel.add(editSeqChoiceText, c);

		
		JLabel editOutgroupLabel = new JLabel("Outgroups:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editOutgroupLabel, c);
		c.gridx++;
		String outgroupStrings[] = {"None", "First in alignment", "Last in alignment", "Custom vector"};
		editOutgroupBox = new JComboBox(outgroupStrings);
		editOutgroupBox.setSelectedIndex(3);
		editOutgroupBox.addItemListener(this);
		parameterPanel.add(editOutgroupBox, c);
		
	   	editOutgroupTextLabel = new JLabel("        Custom vector:");
	   	c.gridy++;
    	c.gridx = 0;
    	parameterPanel.add(editOutgroupTextLabel, c);
		editOutgroupText = new JTextField(30);
		editOutgroupText.addActionListener(this);
		c.gridx++;
		parameterPanel.add(editOutgroupText, c);
		
		JLabel editRunModeLabel = new JLabel("Select RunMode:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editRunModeLabel, c);
		String runModeStrings[] = {"11 - Summary statistics", "12 - Summary statistics (n>3)",
			"21 - Summary statistics with outgroup", "22 - Summary statistics with outgroup (n>2)",
			"31 - LD and haplotype statistics"};
		editRunModeBox = new JComboBox(runModeStrings);
		editRunModeBox.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editRunModeBox, c);
		
		JLabel editUseMutsLabel = new JLabel("Use mutations?");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editUseMutsLabel, c);
		editUseMutsCheck = new JCheckBox();
		editUseMutsCheck.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editUseMutsCheck, c);
		
		JLabel editCompleteDeletionLabel = new JLabel("Complete deletion?");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editCompleteDeletionLabel, c);
		editCompleteDeletionCheck = new JCheckBox();
		editCompleteDeletionCheck.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editCompleteDeletionCheck, c);
		
		editFixNumLabel = new JLabel("        Fix number of nucleotides?");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editFixNumLabel, c);
		editFixNumCheck = new JCheckBox();
		editFixNumCheck.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editFixNumCheck, c);
		
		editNumNucLabel = new JLabel("        Number of nucleotides:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editNumNucLabel, c);
    	editNumNucText = new JTextField(2);
    	editNumNucText.addActionListener(this);
    	c.gridx++;
    	parameterPanel.add(editNumNucText, c);
    	
		JLabel editSlidingWindowLabel = new JLabel("Do sliding window analysis?");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editSlidingWindowLabel, c);
		editSlidingWindowCheck = new JCheckBox("",true);
		editSlidingWindowCheck.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editSlidingWindowCheck, c);
		
		editWidthSwLabel = new JLabel("        Width of window:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editWidthSwLabel, c);
    	editWidthSwText = new JTextField(10);
    	editWidthSwText.addActionListener(this);
    	c.gridx++;
    	parameterPanel.add(editWidthSwText, c);

		editJumpSwLabel = new JLabel("        Jump of window:");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editJumpSwLabel, c);
    	editJumpSwText = new JTextField(10);
    	editJumpSwText.addActionListener(this);
    	c.gridx++;
    	parameterPanel.add(editJumpSwText, c);

    	editWindowTypeLabel = new JLabel("        Window based on:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(editWindowTypeLabel, c);
    	String windowTypeStrings[] = {"Total positions in alignment", 
    		"Net positions in alignment", "Positions in reference sequence",
    		"Polymorphic sites"};
    	editWindowTypeBox = new JComboBox(windowTypeStrings);
    	editWindowTypeBox.addItemListener(this);
		c.gridx++;
    	parameterPanel.add(editWindowTypeBox, c);

		JLabel editUseSingletsLabel = new JLabel("Use singletons for LD analysis?");
		c.gridx = 0;
		c.gridy++;
		parameterPanel.add(editUseSingletsLabel, c);
		editUseSingletsCheck = new JCheckBox();
		editUseSingletsCheck.addItemListener(this);
		c.gridx++;
		parameterPanel.add(editUseSingletsCheck, c);

		JPanel footer = new JPanel();
		footer.setLayout(new BoxLayout(footer, BoxLayout.LINE_AXIS));
		footer.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		footer.add(Box.createHorizontalGlue());		
		editCancelButton = new JButton("Cancel");
		editCancelButton.addActionListener(this);
		footer.add(editCancelButton, c);
		footer.add(Box.createRigidArea(new Dimension(5,0)));
		editSaveButton = new JButton("Save");
		editSaveButton.addActionListener(this);
		footer.add(editSaveButton,c);

   		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.PAGE_AXIS));
		panel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		panel.add(parameterPanel);
		panel.add(footer);
   		
   		return panel;
    }
    
    public Container createLwConfigEditPanel (){
 
    	JPanel parameterPanel = new JPanel(new GridBagLayout());
    	c.anchor = GridBagConstraints.WEST;
		c.fill = GridBagConstraints.NONE;
    	
    	JLabel lwStatColLabel = new JLabel("Statistics column:");
    	c.gridx = c.gridy = 0;
    	parameterPanel.add(lwStatColLabel, c);
    	lwStatColText = new JTextField(2);
    	c.gridx++;
    	parameterPanel.add(lwStatColText, c);
    	
     	JLabel lwRefColLabel = new JLabel("Reference position column:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(lwRefColLabel, c);
    	lwRefColText = new JTextField(2);
    	c.gridx++;
    	parameterPanel.add(lwRefColText, c);
    	
     	JLabel lwSubALabel = new JLabel("Subdivision A:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(lwSubALabel, c);
    	lwSubAText = new JTextField(2);
    	c.gridx++;
    	parameterPanel.add(lwSubAText, c);
    	
     	JLabel lwSubBLabel = new JLabel("Subdivision B:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(lwSubBLabel, c);
    	lwSubBText = new JTextField(2);
    	c.gridx++;
    	parameterPanel.add(lwSubBText, c);
    	
     	JLabel lwSubCLabel = new JLabel("Subdivision C:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(lwSubCLabel, c);
    	lwSubCText = new JTextField(2);
    	c.gridx++;
    	parameterPanel.add(lwSubCText, c);
    	
     	JLabel lwFilterLabel = new JLabel("Filter:");
    	c.gridx = 0;
    	c.gridy++;
    	parameterPanel.add(lwFilterLabel, c);
    	lwFilterText = new JTextField(10);
    	c.gridx++;
    	parameterPanel.add(lwFilterText, c);
    	
		JPanel footer = new JPanel();
		footer.setLayout(new BoxLayout(footer, BoxLayout.LINE_AXIS));
		footer.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		footer.add(Box.createHorizontalGlue());		
		lwCancelButton = new JButton("Cancel");
		lwCancelButton.addActionListener(this);
		footer.add(lwCancelButton, c);
		footer.add(Box.createRigidArea(new Dimension(5,0)));
		lwSaveButton = new JButton("Save");
		lwSaveButton.addActionListener(this);
		footer.add(lwSaveButton,c);

   		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.PAGE_AXIS));
		panel.setBorder(BorderFactory.createEmptyBorder(5,5,5,5));
		panel.add(parameterPanel);
		panel.add(footer);
   		
   		return panel;
    }

    public void actionPerformed(ActionEvent e) {
		if (e.getSource().getClass().getName() == "javax.swing.JMenuItem") {
			// We are clicking on the menu, so go fetch the right card
			JMenuItem source = (JMenuItem)e.getSource();
        	CardLayout cl = (CardLayout)(mainPanel.getLayout());
        	cl.show(mainPanel, source.getAccessibleContext().
        		getAccessibleName());
		}
		
		//Buttons in the settings part
		else if (e.getSource() == settingsVsBinButton) {
			// go look for the binary file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			vsBinFile = fc.getSelectedFile();
			settingsVsBinText.setText(vsBinFile.getPath());
		}
		
		else if (e.getSource() == settingsLwBinButton) {
			// go look for the binary file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			lwBinFile = fc.getSelectedFile();
			settingsLwBinText.setText(lwBinFile.getPath());
		}
		
		else if (e.getSource() == settingsLwDirButton) {
			// go look for the lw directory
			fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			lwDir = fc.getSelectedFile();
			settingsLwDirText.setText(lwDir.getPath());
		}
		
		else if (e.getSource() == settingsLwSourceDirButton) {
			// go look for the lwSourceDirs directory
			fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			lwSourceDir = fc.getSelectedFile();
			settingsLwSourceDirText.setText(lwSourceDir.getPath());
		}
		
		else if (e.getSource() == settingsScriptDirButton) {
			// go look for the scripts directory
			fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			scriptDir = fc.getSelectedFile();
			settingsScriptDirText.setText(scriptDir.getPath());
		}
		
		else if (e.getSource() == settingsSaveButton) {
			// save settings to file
			if (!settingsFile.exists()) {
				try {
					settingsFile.createNewFile();
				}
				catch (IOException ioe) {
					createFailed("VSsettings.ini");
				}
			}
			writeSettingsFile(settingsFile);
		}

		//Buttons in the VariScan part
		else if (e.getSource() == vsAliButton) {
			// go look for the binary file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			vsAliFile = fc.getSelectedFile();
			vsAliText.setText(vsAliFile.getPath());
		}
		
		else if (e.getSource() == vsConfigButton) {
			// go look for the config file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			File testFile = fc.getSelectedFile();
			//Check If it is OK
			if (readVsConfigFile(testFile) != 18) {
				JFrame frame = new JFrame();
				JOptionPane.showMessageDialog(frame,
					"This VariScan config file is not valid! \n" + 
					"It doesn't contain all required parameters!",
					"Error in config file",
					JOptionPane.ERROR_MESSAGE);
				return;
			}
			vsConfigFile = testFile;
			vsConfigText.setText(vsConfigFile.getPath());
			checkVsConfigConflicts();
		}
		
		else if (e.getSource() == vsConfigCreateButton) {
			//create a new config file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showSaveDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			File testFile = fc.getSelectedFile();
			if (!testFile.exists()) {
				try {
					testFile.createNewFile();
				}
				catch (IOException ioe) {
					createFailed("config file");
				}
			}
			else {
				if (!confirmOverwrite()) {
					return;
				}
			}
			
			vsConfigFile = testFile;
			setDefaultVsConfigValues();
			writeVsConfigFile(vsConfigFile);
			vsConfigText.setText(vsConfigFile.getPath());
		}

		else if (e.getSource() == vsConfigEditButton) {
			//in case somebody edited the field
			if (vsConfigFile.exists()) {
				//read again, just to be sure 
				readVsConfigFile(vsConfigFile);
				
				vsConfigEditFrame = new JFrame("VariScan Config Editor");
				vsConfigEditFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
				JPanel vsConfigEditPanel = new JPanel();
				vsConfigEditFrame.setContentPane(createVsConfigEditPanel());
			
				setVsConfigEditFields();

				vsConfigEditFrame.pack();
				vsConfigEditFrame.setVisible(true);
			}
		}
		
		else if (e.getSource() == vsOutputButton) {
			// go look for the scripts directory
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showSaveDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			File testFile = fc.getSelectedFile();
			if (testFile.exists()) {
				if (!confirmOverwrite()) {
					return;
				}
			}
			vsOutputFile = testFile;
			vsOutputText.setText(vsOutputFile.getPath());
			
			// auto-update the inputs and outputs for the other parts
			if (lwEnableCheck.isSelected()) {
				chainVstoLw();
				if (visEnableCheck.isSelected()) {
					chainLwtoVis();
				}
			}
		}
		
		//Buttons in the LastWave part
		else if (e.getSource() == lwInputButton) {
			// go look for the input file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			lwInputFile = fc.getSelectedFile();
			lwInputText.setText(lwInputFile.getPath());

			// auto-set the output file
			autosetLwOutputFile();
						
			// auto-update the inputs and outputs for the other parts
			if (visEnableCheck.isSelected()) {
				chainLwtoVis();
			}
		}
		
		else if (e.getSource() == lwConfigButton) {
			// go look for the config file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			File testFile = fc.getSelectedFile();
			lwConfigText.setText(lwConfigFile.getPath());
			//test if it is OK
			if (!checkLwSettings()) {
				//the function will spit out an error dialog	
			}
			else if (readLwConfigFile(testFile) != 6) {
				JFrame frame = new JFrame();
				JOptionPane.showMessageDialog(frame,
					"This LastWave config file is not valid! \n" + 
					"It doesn't contain all required parameters!",
					"Error in config file",
					JOptionPane.ERROR_MESSAGE);
			}
			lwConfigFile = testFile;
			lwConfigText.setText(lwConfigFile.getPath());
			checkLwConfigConflicts();
		}
		
		else if (e.getSource() == lwConfigCreateButton) {
			//create a new config file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showSaveDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			File testFile = fc.getSelectedFile();
			if (!testFile.exists()) {
				try {
					testFile.createNewFile();
				}
				catch (IOException ioe) {
					createFailed("config file");
				}
			}
			else {
				if (!confirmOverwrite()) {
					return;
				}
			}
			
			lwConfigFile = testFile;
			setDefaultLwConfigValues();
			writeLwConfigFile(lwConfigFile);
			lwConfigText.setText(lwConfigFile.getPath());
		}
		
		else if (e.getSource() == lwConfigEditButton) {
			if (lwConfigFile.exists()) {
				//read again, just to be sure 
				readLwConfigFile(lwConfigFile);
				
				lwConfigEditFrame = new JFrame("LastWave Config Editor");
				lwConfigEditFrame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
				JPanel lwConfigEditPanel = new JPanel();
				lwConfigEditFrame.setContentPane(createLwConfigEditPanel());
				
				setLwConfigEditFields();
			
				lwConfigEditFrame.pack();
				lwConfigEditFrame.setVisible(true);
			}
		}
		
		//Buttons in the visualization part
		else if (e.getSource() == visInputButton) {
			// go look for the input file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			visInputFile = fc.getSelectedFile();
			visInputText.setText(visInputFile.getPath());

			// auto-set the output file
			autosetVisOutputFile();
		}
		
		//Buttons in the config Editors
		else if (e.getSource() == editBdfFileButton) {
			// go look for the config file
			fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {return;}
			vsBdfFile = fc.getSelectedFile();
			editBdfFileText.setText(vsBdfFile.getPath());
		}
		
		else if (e.getSource() == editCancelButton) {
			vsConfigEditFrame.dispose();
		}
		
		else if (e.getSource() == editSaveButton) {
			if(setVsConfigValues()) {
				if (writeVsConfigFile(vsConfigFile)) {
					vsConfigEditFrame.dispose();
				}
			}
		}
		
		else if (e.getSource() == lwSaveButton) {
			if(setLwConfigValues()) {
				if (writeLwConfigFile(lwConfigFile)) {
					lwConfigEditFrame.dispose();
				}
			}
		}
		
		else if (e.getSource() == lwCancelButton) {
			lwConfigEditFrame.dispose();
		}
		
		
		// RUN!
		else if (e.getSource() == analysisRunButton) {	
			if (visEnableCheck.isSelected()) {
				String checkChrom = visChromIdText.getText().trim();
				if (checkChrom.equals("")){
					JFrame frame = new JFrame();
					JOptionPane.showMessageDialog(frame,
						"Please enter a value for Chromosome ID in the \n" +
						"visualization parameters!\n",
						"Chromosome ID missing",
						JOptionPane.ERROR_MESSAGE);
					return;
				}
			}
			runAnalysis();
   		}
   	}
    
    public void itemStateChanged(ItemEvent e) {
    	boolean status;
    	String statusText;
    	// Enable/Disable stuff in the analysis panel
    	if (e.getSource() == vsEnableCheck) {
    		status = vsEnableCheck.isSelected();
    		Component[] components = vsPanel.getComponents();
    		for (int i = 0; i < components.length; i++) {
    			components[i].setEnabled(status);
    		}
    	
    	}
    	
    	else if (e.getSource() == lwEnableCheck) {
    		status = lwEnableCheck.isSelected();
    		Component[] components = lwPanel.getComponents();
    		for (int i = 0; i < components.length - 1 ; i++) {
    			components[i].setEnabled(status);
    		}
    	
    	}
    	
    	else if (e.getSource() == visEnableCheck) {
    		status = visEnableCheck.isSelected();
    		Component[] components = visPanel.getComponents();
    		for (int i = 0; i < components.length; i++) {
    			components[i].setEnabled(status);
    		}
    		//the output file button is always disabled
    		components[5].setEnabled(false);
    	}
    	
    	//switch the visualization types
    	else if (e.getSource() == visTypeBox) {
    		visType = visTypeBox.getSelectedIndex();
    		autosetVisOutputFile();
    	}	
    	
    	// Enable/Disable stuff in the VS config editor
    	else if (e.getSource() == editBdfFileCheck) {
    		 status = editBdfFileCheck.isSelected();
    		 editBdfFileTextLabel.setEnabled(status);
    		 editBdfFileText.setEnabled(status);
    		 editBdfFileButton.setEnabled(status);
    	}
    	
    	else if (e.getSource() == editSeqChoiceBox) {
    		if (editSeqChoiceBox.getSelectedItem() == "All") {
    			status = false;
    		}
    		else {
    			status = true;
    		}
    		editSeqChoiceTextLabel.setEnabled(status);
    		editSeqChoiceText.setEnabled(status);
    		
    	}
    	
    	else if (e.getSource() == editOutgroupBox) {
    		if (editOutgroupBox.getSelectedItem() == "Custom vector") {
    			status = true;
    		}
    		else {
    			status = false;
    		}
    		editOutgroupTextLabel.setEnabled(status);
    		editOutgroupText.setEnabled(status);
    		
    	}
    	
    	else if (e.getSource() == editRunModeBox) {
    		if (editRunModeBox.getSelectedIndex() == 4) {
    			status = true;
    			statusText = "Mandatory!";
    		}
    		else {
    			status = false;
    			statusText = "";
    		}
    		editSlidingWindowCheck.setEnabled(!status);
    		editSlidingWindowCheck.setSelected(status);
    		editSlidingWindowCheck.setText(statusText);
    		
    		editCompleteDeletionCheck.setEnabled(!status);
    		editCompleteDeletionCheck.setSelected(status);
    		editCompleteDeletionCheck.setText(statusText);
    		
    	}
    	
    	else if (e.getSource() == editCompleteDeletionCheck) {
    		 status = editCompleteDeletionCheck.isSelected();
    		 editFixNumLabel.setEnabled(!status);
    		 editFixNumCheck.setEnabled(!status);
    		 editNumNucLabel.setEnabled(!status);
    		 editNumNucText.setEnabled(!status);
    	}
    	
    	else if (e.getSource() == editSlidingWindowCheck) {
    		 status = editSlidingWindowCheck.isSelected();
    		 editWidthSwLabel.setEnabled(status);
    		 editWidthSwText.setEnabled(status);
    		 editJumpSwLabel.setEnabled(status);
    		 editJumpSwText.setEnabled(status);
    		 editWindowTypeLabel.setEnabled(status);
    		 editWindowTypeBox.setEnabled(status);
    	}
    	
    }
    
    public void runAnalysis () {
		long start = System.currentTimeMillis();
		long end = 0;
		
		//get the OS
		String os = System.getProperty("os.name").toLowerCase();
		if (os.startsWith("windows")) {
			if (lwEnableCheck.isSelected()) {
				//won't work
				JFrame frame = new JFrame();
				JOptionPane.showMessageDialog(frame,
					"Since there is no native Windows build of LastWave, \n" +
					"it cannot be called by the GUI. Please disable the \n" +
					"\"Perform LastWave analysis\" checkbox. \n" +
					"You can run LastWave manually from within the \n" + 
					"CYGWIN environment. Get it at: www.cygwin.com\n\n" + 
					"Please consult the VariScan and LastWave manuals for Details",
					"Windows OS detected",
					JOptionPane.ERROR_MESSAGE);
				return;
			}
			
			shell = "cmd.exe";
			shellParam = "/c";
			perlBin = "perl.exe";
		}
		else {
			//let's assume OS X, Linux/Unix and all the rest are the same
			shell = "sh";
			shellParam = "-c";
			perlBin = "perl";
		}
			
		
		//Run VariScan
		if (vsEnableCheck.isSelected()) {
			statusLabel.setText("Status: VariScan running");
			progress.setIndeterminate(true); 
			runVs();
		}
		
		//Run LastWave
		if (lwEnableCheck.isSelected()) {
			statusLabel.setText("Status: LastWave running");
			progress.setIndeterminate(true); 
			runLw();
		}
		
		//Run visualization
		if (visEnableCheck.isSelected()) {
			statusLabel.setText("Status: Visualization running");
			progress.setIndeterminate(true); 
			runVis();
		}
		
		//everything is done, set all back to normal
		progress.setIndeterminate(false);
		statusLabel.setText("Status: Ready");
		end = System.currentTimeMillis();
		long time = (end - start)/1000;
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"All analyses have been completed!\n" + 
			"Time: " + time + " seconds.",
			"Analysis complete",
			JOptionPane.INFORMATION_MESSAGE);

		
		// save this run
		if (!recentFile.exists()) {
			try {
				recentFile.createNewFile();
			}
			catch (IOException ioe) {
				createFailed("VSrecent.ini");
			}
		}
		writeRecentFile(recentFile);
	}
	
	public void runVs () {
		String[] command;
        String error = "";
		/*
		if (!vsOutputFile.exists()) {
			try {
				vsOutputFile.createNewFile();
			}
			catch (IOException ioe){
			}
		}
		*/
		try {
			String vsString = vsBinFile.getPath() + " " + vsAliFile.getPath() + 
				" " + vsConfigFile.getPath() + " >" + vsOutputFile.getPath();
				
			command = new String[] {shell, shellParam, vsString};
			
			Process p = Runtime.getRuntime().exec(command);
			
        	error = getErrorStream(p);
        	
        	/*
			InputStream vsIn = p.getInputStream();
        	InputStreamReader vsInReader = new InputStreamReader(vsIn);
        	BufferedReader vsInBuffer = new BufferedReader(vsInReader);
        	
			FileWriter vsOut = new FileWriter(vsOutputFile);
			BufferedWriter vsOutBuffer = new BufferedWriter(vsOut);
        	while ((line = vsInBuffer.readLine()) != null) {
            	vsOutBuffer.write(line);
            	vsOutBuffer.newLine();
       		}				
			vsIn.close();
			vsOut.close();
			*/
			p.waitFor();
		}
		catch (Exception exc) {
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"Couldn't run VariScan. Make sure you have \n" +
			"executable rights for the binary and write \n" +
			"rights for the output file!",
			"Error while running VariScan",
			JOptionPane.ERROR_MESSAGE);
		return;
		}
		
		//check if the output is OK, can't use the error stream
		//if (!error.equals("")) {
		if (!vsOutputFile.exists() || vsOutputFile.length() == 0) {
			JFrame frame = new JFrame();
			JOptionPane.showMessageDialog(frame,
			"VariScan returned an error!\n" +
			"The shell was invoked with:\n" + 
			shell + " " + shellParam + "\n" + 
			"The called command in the shell was:\n" + 
			vsBinFile.getPath() + "\n" +
			vsAliFile.getPath() + "\n" +
			vsConfigFile.getPath() + "\n" +
			"> " + vsOutputFile.getPath() + "\n\n" + 
			"Error output of VariScan was:\n" +
			error,
			"Error while running VariScan",
			JOptionPane.ERROR_MESSAGE);
		}
	}
	
	public void runLw () {
		String[] command;
		String error = "";
		File script;

		try {
			script = new File(scriptDir.getAbsoluteFile(), "runMRA.PLS");
			
			//old command
			//command = new String[] {perlBin, script.getPath(), 
			//	lwInputFile.getPath(), lwConfigFile.getPath()};
			
			String lwString = perlBin + " " +  script.getPath() + " " + 
				lwInputFile.getPath() + " " + lwConfigFile.getPath();
			
			command = new String[] {shell, shellParam, lwString};
			
			String[] envr = new String[] {"LWPATH="+lwDir.getPath(), 
				"LWSOURCEDIR="+lwSourceDir.getPath()};
			
			//environment variables don't work here (why?)
			Process p = Runtime.getRuntime().exec(command, null, 
				lwInputFile.getParentFile());
			
			error = getErrorStream(p);
			
			p.waitFor();
		}
		catch (Exception exc) {
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"Couldn't run LastWave. Make sure you have \n" +
			"PERL installed on your system and the \n" + 
			"selected files still exist!",
			"Error while running LastWave",
			JOptionPane.ERROR_MESSAGE);
		return;
		}
		
		//check if the output is OK
		if (!error.equals("")) {
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"LastWave returned an error!\n" +
			"The shell was invoked with:\n" + 
			shell + " " + shellParam + "\n" + 
			"The called command in the shell was:\n" + 
			perlBin + "\n" +
			script.getPath() + "\n" +
			lwInputFile.getPath() + "\n" +
			lwConfigFile.getPath() + "\n\n" + 
			"Error output was:\n" +
			error,
			"Error while running LastWave",
			JOptionPane.ERROR_MESSAGE);
		}
	}
           
    public void runVis () {
		String[] command;
		String error = "";
		File script;
		String chrom;
		
		try {
			
			String scriptType;
			if (visType == 0) {
				scriptType = "mra2bed.PLS";
			}
			else {
				scriptType = "mra2gbrowse.PLS";
			}
			
			script = new File(scriptDir.getAbsoluteFile(), scriptType);
			chrom = "-chr chr" + visChromIdText.getText();
			
			
			String visString = perlBin +  " " + script.getPath() + " " +  
				visInputFile.getPath() + " " + chrom;
				
			command = new String[] {shell, shellParam, visString};
			
			Process p = Runtime.getRuntime().exec(command,null,
				visInputFile.getParentFile());
			
			error = getErrorStream(p);
			
			p.waitFor(); 
		}
		catch (Exception exc) {
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"Couldn't create visualization. Make sure \n" +
			"you have PERL installed on your system \n" +
			"and the selected files still exist!",
			"Error while creating Visualization",
			JOptionPane.ERROR_MESSAGE);
		return;
		}
		
		//check if the output is OK
		if (!error.equals("")) {
		JFrame frame = new JFrame();
		JOptionPane.showMessageDialog(frame,
			"Visualization returned an error!\n" +
			"The shell was invoked with:\n" + 
			shell + " " + shellParam + "\n" + 
			"The called command in the shell was:\n" + 
			perlBin + "\n" +
			script.getPath() + "\n" +
			visInputFile.getPath() + "\n" +
			chrom + "\n\n" + 
			"Error output of visualization script was:\n" +
			error,
			"Error while running Visualization",
			JOptionPane.ERROR_MESSAGE);
		}
    }
    
    public String getErrorStream (Process p) throws IOException {
		InputStream err = p.getErrorStream();
    	InputStreamReader errReader = new InputStreamReader(err);
    	BufferedReader errBuffer = new BufferedReader(errReader);
    	
    	String error = "";;
    	String line;
    	while((line = errBuffer.readLine()) != null) {
    		error = error.concat(line + "\n");
    	}
		
		err.close();
		return error;
    }
    
    public void readSettingsFile () {
		try {
			BufferedReader settingsIn = new BufferedReader(new FileReader(settingsFile));
			while ((line = settingsIn.readLine()) != null) {
				if (line.startsWith("scriptDir=")) {
					line = line.replaceFirst("scriptDir=","");
					scriptDir = new File(line);
					settingsScriptDirText.setText(scriptDir.getPath());
				}
				else if (line.startsWith("vsBinFile=")) {
					line = line.replaceFirst("vsBinFile=","");
					vsBinFile = new File(line);
					settingsVsBinText.setText(vsBinFile.getPath());
				}
				else if (line.startsWith("lwDir=")) {
					line = line.replaceFirst("lwDir=","");
					lwDir = new File(line);
					settingsLwDirText.setText(lwDir.getPath());
				}
				else if (line.startsWith("lwSourceDir=")) {
					line = line.replaceFirst("lwSourceDir=","");
					lwSourceDir = new File(line);
					settingsLwSourceDirText.setText(lwSourceDir.getPath());
				}
				else if (line.startsWith("lwBinFile=")) {
					line = line.replaceFirst("lwBinFile=","");
					lwBinFile = new File(line);
					settingsLwBinText.setText(lwBinFile.getPath());
				}
			}
			settingsIn.close();
		}
		catch (IOException ioe) {
			readFailed("VSsettings.ini");
		}
    }
    
    public void writeSettingsFile (File file) {
    	try {
			BufferedWriter settingsOut = new BufferedWriter(new FileWriter(file));
			//The settings for the binaries and scripts
			settingsOut.write("scriptDir=" + scriptDir.getPath());
			settingsOut.newLine();
			settingsOut.write("vsBinFile=" + vsBinFile.getPath());
			settingsOut.newLine();
			settingsOut.write("lwDir=" + lwDir.getPath());
			settingsOut.newLine();
			settingsOut.write("lwSourceDir=" + lwSourceDir.getPath());
			settingsOut.newLine();
			settingsOut.write("lwBinFile=" + lwBinFile.getPath());
			settingsOut.newLine();
			settingsOut.close();
		}
		catch (IOException ioe) {
			writeFailed("settings to \"VSsettings.ini\"");
			return;
		}
		writeSuccessful("settings file");
    }
    
    public void readRecentFile () {
		try {
			BufferedReader recentIn = new BufferedReader(new FileReader(recentFile));
			while ((line = recentIn.readLine()) != null) {
				//variscan
				if (line.startsWith("vsAliFile=")) {
					line = line.replaceFirst("vsAliFile=","");
					vsAliFile = new File(line);
					vsAliText.setText(vsAliFile.getPath());
				}
				else if (line.startsWith("vsConfigFile=")) {
					line = line.replaceFirst("vsConfigFile=","");
					vsConfigFile = new File(line);
					vsConfigText.setText(vsConfigFile.getPath());
				}
				else if (line.startsWith("vsOutputFile=")) {
					line = line.replaceFirst("vsOutputFile=","");
					vsOutputFile = new File(line);
					vsOutputText.setText(vsOutputFile.getPath());
				}
				
				//lastwave
				else if (line.startsWith("lwInputFile=")) {
					line = line.replaceFirst("lwInputFile=","");
					lwInputFile = new File(line);
					lwInputText.setText(lwInputFile.getPath());
				}
				else if (line.startsWith("lwConfigFile=")) {
					line = line.replaceFirst("lwConfigFile=","");
					lwConfigFile = new File(line);
					lwConfigText.setText(lwConfigFile.getPath());
				}
				else if (line.startsWith("lwOutputFile=")) {
					line = line.replaceFirst("lwOutputFile=","");
					lwOutputFile = new File(line);
					lwOutputText.setText(lwOutputFile.getPath());
				}
				
				//visualization
				else if (line.startsWith("visInputFile=")) {
					line = line.replaceFirst("visInputFile=","");
					visInputFile = new File(line);
					visInputText.setText(visInputFile.getPath());
				}
				else if (line.startsWith("visOutputFile=")) {
					line = line.replaceFirst("visOutputFile=","");
					visOutputFile = new File(line);
					visOutputText.setText(visOutputFile.getPath());
				}
				else if (line.startsWith("chromId=")) {
					line = line.replaceFirst("chromId=","");
					visChromIdText.setText(line);
				}
				else if (line.startsWith("visType=")) {
					line = line.replaceFirst("visType=","");
					visType = Integer.parseInt(line);
					visTypeBox.setSelectedIndex(visType);
				}

				//enabled analysis
				else if (line.startsWith("vsEnable=")) {
					line = line.replaceFirst("vsEnable=","");
					vsEnableCheck.setSelected(Boolean.valueOf(line).booleanValue());
				}
				else if (line.startsWith("lwEnable=")) {
					line = line.replaceFirst("lwEnable=","");
					lwEnableCheck.setSelected(Boolean.valueOf(line).booleanValue());
				}
				else if (line.startsWith("visEnable=")) {
					line = line.replaceFirst("visEnable=","");
					visEnableCheck.setSelected(Boolean.valueOf(line).booleanValue());
				}
			}
			recentIn.close();
		}
		catch (IOException ioe) {
			readFailed("VSrecent.ini");
		}
    }
    
    public void writeRecentFile (File file) {
    	try {
			BufferedWriter recentOut = new BufferedWriter(new FileWriter(file));
			//The files used for the most recent run
			recentOut.write("vsAliFile=" + vsAliFile.getPath());
			recentOut.newLine();
			recentOut.write("vsConfigFile=" + vsConfigFile.getPath());
			recentOut.newLine();
			recentOut.write("vsOutputFile=" + vsOutputFile.getPath());
			recentOut.newLine();
			recentOut.write("lwInputFile=" + lwInputFile.getPath());
			recentOut.newLine();
			recentOut.write("lwConfigFile=" + lwConfigFile.getPath());
			recentOut.newLine();
			recentOut.write("lwOutputFile=" + lwOutputFile.getPath());
			recentOut.newLine();
			recentOut.write("visInputFile=" + visInputFile.getPath());
			recentOut.newLine();
			recentOut.write("visOutputFile=" + visOutputFile.getPath());
			recentOut.newLine();
			recentOut.write("vsEnable=" + String.valueOf(vsEnableCheck.isSelected()));
			recentOut.newLine();
			recentOut.write("lwEnable=" + String.valueOf(lwEnableCheck.isSelected()));
			recentOut.newLine();
			recentOut.write("visEnable=" + String.valueOf(visEnableCheck.isSelected()));
			recentOut.newLine();
			recentOut.write("chromId=" + visChromIdText.getText());
			recentOut.newLine();
			recentOut.write("visType=" + String.valueOf(visType));
			recentOut.newLine();
			recentOut.close();
		}
		catch (IOException ioe) {
			writeFailed("used files to VSrecent.ini");
		}
    }
        
    public int readVsConfigFile (File file) {
    	int readParams = 0;
    	try {
    		BufferedReader configIn = new BufferedReader(new FileReader(file));
    		String concat;
    		while ((line = configIn.readLine()) != null) {
    			line = line.trim();
    			if (line.startsWith("#") || line.equals("")) {
    				continue;
    			}
    			
    			String longString = "";
    			String[] words = line.split("\\s+");
    			
    			if (words[0].equals("StartPos")) {
    				startPos = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("EndPos")) {
    				endPos = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("RefPos")) {
    				refPos = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("BlockDataFile")) {
    				if (words.length == 2) {
    					bdfFile = "";
    				}
    				else {
    					/*could be whitespace in there (will the NOT work with
    					 *VariScan!!!
    					 */
    					for (int i=2;i<words.length;i++) {
    						longString = longString.concat(words[i]).concat(" ");
    					}
    					longString = longString.trim();
    					if (longString.equals("none")) {
    						bdfFile = "";
    					}
    					else {
 		   					bdfFile = longString;
    					}
    				}
    				readParams++;
    			}
    			else if (words[0].equals("IndivNames")) {
    				if (words.length == 2) {
    					indivNames = "";
    				}
    				else {
    					for (int i=2;i<words.length;i++) {
    						longString = longString.concat(words[i]).concat(" ");
    					}
    					longString = longString.trim();
    					indivNames = longString;
    				}
    				readParams++;
    			}
    			else if (words[0].equals("SeqChoice")) {
    				for (int i=2;i<words.length;i++) {
    					longString = longString.concat(words[i]).concat(" ");
    				}
    				longString = longString.trim();
    				seqChoice = longString;
    				readParams++;
    			}
    			else if (words[0].equals("Outgroup")) {
    				for (int i=2;i<words.length;i++) {
    					longString = longString.concat(words[i]).concat(" ");
    				}
    				longString = longString.trim();
    				outgroup = longString;
    				readParams++;
    			}
    			else if (words[0].equals("RefSeq")) {
    				refSeq = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("RunMode")) {
    				runMode = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("UseMuts")) {
    				useMuts = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("CompleteDeletion")) {
    				completeDeletion = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("FixNum")) {
    				fixNum = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("NumNuc")) {
    				numNuc = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("SlidingWindow")) {
    				slidingWindow = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("WidthSW")) {
    				widthSW = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("JumpSW")) {
    				jumpSW = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("WindowType")) {
    				windowType = words[2];
    				readParams++;
    			}
    			else if (words[0].equals("UseLDSinglets")) {
    				useLdSinglets = words[2];
    				readParams++;
    			}
    		}
    	}
    	catch (IOException ioe) {
    		readFailed("the config file");
    	}
    	return readParams;
    }
    
    public void setVsConfigEditFields () {
    	editStartPosText.setText(startPos);
    	editEndPosText.setText(endPos);
    	
    	if (refPos.equals("1")) {
    		editRefPosBox.setSelectedItem("Reference sequence");
    	}
    	else if (refPos.equals("0")) {
    		editRefPosBox.setSelectedItem("Alignment");
    	}
    	else {
    		//here comes a "corrupt config" dialog
    	}
    	
    	editRefSeqText.setText(refSeq);
    	
    	if (bdfFile.equals("")) {
    		editBdfFileCheck.setSelected(false);
    		editBdfFileText.setText(bdfFile);
    	}
    	else {
    		editBdfFileCheck.setSelected(true);
    		editBdfFileText.setText(bdfFile);
    	}
    	
    	editIndivNamesText.setText(indivNames);
    	
    	if (seqChoice.equals("all")) {
    		editSeqChoiceBox.setSelectedIndex(0);
    		editSeqChoiceText.setText("");
    	}
		else {
    		editSeqChoiceBox.setSelectedIndex(1);
    		editSeqChoiceText.setText(seqChoice);
		}
    	
    	if (outgroup.equals("none")) {
    		editOutgroupBox.setSelectedIndex(0);
    		editOutgroupText.setText("");
    	}
    	else if (outgroup.equals("first")){
    		editOutgroupBox.setSelectedIndex(1);
    		editOutgroupText.setText("");
    	}
    	else if (outgroup.equals("last")){
    		editOutgroupBox.setSelectedIndex(2);
    		editOutgroupText.setText("");
    	}
    	else {
    		editOutgroupBox.setSelectedIndex(3);
    		editOutgroupText.setText(outgroup);
    	}
    	
    	if (runMode.equals("11")) {
    		editRunModeBox.setSelectedIndex(0);
    	}
    	else if (runMode.equals("12")) {
    		editRunModeBox.setSelectedIndex(1);
    	}
    	else if (runMode.equals("21")) {
    		editRunModeBox.setSelectedIndex(2);
    	}
    	else if (runMode.equals("22")) {
    		editRunModeBox.setSelectedIndex(3);
    	}
    	else if (runMode.equals("31")) {
    		editRunModeBox.setSelectedIndex(4);
    	}
    	else {
    		//corrupt
    	}
    	
    	if (useMuts.equals("1")) {
    		editUseMutsCheck.setSelected(true);
    	}
    	else if (useMuts.equals("0")) {
    		editUseMutsCheck.setSelected(false);
    	}
    	else {
    		//corrupt
    	}
    	
    	if (completeDeletion.equals("1")) {
    		editCompleteDeletionCheck.setSelected(true);
    	}
    	else if (completeDeletion.equals("0")) {
    		/* If stupid users create a config file with runMode=31 and
    		 * completeDeletion=0, this could screw up things on loading.
    		 * Take care of that here */
    		if (editRunModeBox.getSelectedIndex() != 4) {
    			editCompleteDeletionCheck.setSelected(false);
    		}
    	}
    	else {
    		//corrupt
    	}
    	
    	if (fixNum.equals("1")) {
    		editFixNumCheck.setSelected(true);
    	}
    	else if (fixNum.equals("0")) {
    		editFixNumCheck.setSelected(false);
    	}
    	else {
    		//corrupt
    	}
    	
    	editNumNucText.setText(numNuc);
    	
    	if (slidingWindow.equals("1")) {
    		editSlidingWindowCheck.setSelected(true);
    	}
    	else if (slidingWindow.equals("0")) {
    		/* If stupid users create a config file with runMode=31 and
    		 * slidingWindow=0, this could screw up things on loading.
    		 * Take care of that here */
    		if (editRunModeBox.getSelectedIndex() != 4) {
    			editSlidingWindowCheck.setSelected(false);
    		}
    	}
    	else {
    		//corrupt
    	}
    	
    	editWidthSwText.setText(widthSW);
    	editJumpSwText.setText(jumpSW);
    	
    	int i = Integer.valueOf(windowType).intValue();
    	if (0 <= i && 4 >= i) {
    			editWindowTypeBox.setSelectedIndex(i);
    	}
    	else {
    		//corrupt
    	}
    	
    	if (useLdSinglets.equals("1")) {
    		editUseSingletsCheck.setSelected(true);
    	}
    	if (useLdSinglets.equals("0")) {
    		editUseSingletsCheck.setSelected(false);
    	}
    	else {
    		//corrupt
    	}
    }
    
    public int readLwConfigFile (File file) {
    	int readParams = 0;
    	try {
    		BufferedReader configIn = new BufferedReader(new FileReader(file));
    		while ((line = configIn.readLine()) != null) {
    			line = line.trim();
    			if (line.startsWith("#") || line == "") {
    				continue;
    			}
    			
    			String[] words = line.split("\\s+");
				if (words[0].equals("stat_col")) {
    				lwStatCol = words[2];
    				readParams++;
    			}
				else if (words[0].equals("ref_col")) {
    				lwRefCol = words[2];
    				readParams++;
    			}
				else if (words[0].equals("sub_a")) {
    				lwSubA = words[2];
    				readParams++;
    			}
				else if (words[0].equals("sub_b")) {
    				lwSubB = words[2];
    				readParams++;
    			}
				else if (words[0].equals("sub_c")) {
    				lwSubC = words[2];
    				readParams++;
    			}
				else if (words[0].equals("filter")) {
    				lwFilter = words[2];
    				readParams++;
    			}
    		}
    	}
    	catch (IOException ioe) {
    		readFailed("the config file");	
    	}
    	return readParams;
    }
    
    public void setLwConfigEditFields () {
    	
    	lwStatColText.setText(lwStatCol);
    	lwRefColText.setText(lwRefCol);
    	lwSubAText.setText(lwSubA);
    	lwSubBText.setText(lwSubB);
    	lwSubCText.setText(lwSubC);
    	lwFilterText.setText(lwFilter);
    }
    
    public void setDefaultVsConfigValues () {
    	startPos = "1";
    	endPos = "0";
    	refPos = "0";
    	bdfFile = "";
    	indivNames = "";
    	seqChoice = "all";
    	outgroup = "none";
    	refSeq = "1";
    	runMode = "11";
    	useMuts = "1";
    	completeDeletion = "1";
    	fixNum = "1";
    	numNuc = "4";
    	slidingWindow = "1";
    	widthSW = "100";
    	jumpSW = "100";
    	windowType = "0";
    	useLdSinglets = "1";
    }
    
    public boolean setVsConfigValues() {
    	
    	startPos = editStartPosText.getText();
    	endPos = editEndPosText.getText();
    	refPos = Integer.toString(editRefPosBox.getSelectedIndex());
    	refSeq = editRefSeqText.getText();
    	
    	if (editBdfFileCheck.isSelected()) {
    		bdfFile = editBdfFileText.getText();
    	}
    	else {
    		bdfFile = "none";
    	}
    	
    	indivNames = editIndivNamesText.getText();
    	
    	if (editSeqChoiceBox.getSelectedIndex() == 0) {
    		seqChoice = "all";
    	}
    	else {
    		seqChoice = editSeqChoiceText.getText();
    	}
    	
    	if (editOutgroupBox.getSelectedIndex() == 0) {
    		outgroup = "none";
    	}
    	else if (editOutgroupBox.getSelectedIndex() == 1) {
    		outgroup = "first";
    	}
    	else if (editOutgroupBox.getSelectedIndex() == 2) {
    		outgroup = "last";
    	}
    	else {
    		outgroup = editOutgroupText.getText();
    	}
    	
    	switch (editRunModeBox.getSelectedIndex()) {
    		case 0: runMode = "11";
    				break;
    		case 1: runMode = "12";
    				break;
    		case 2: runMode = "21";
    				break;
    		case 3: runMode = "22";
    				break;
    		case 4: runMode = "31";
    				break;

    	}
    	
    	if (editUseMutsCheck.isSelected()) {
    		useMuts = "1";
    	}
    	else {
    		useMuts = "0";
    	}
    	
    	if (editCompleteDeletionCheck.isSelected()) {
    		completeDeletion = "1";
    	}
    	else {
    		completeDeletion = "0";
    	}
    	
    	if (editFixNumCheck.isSelected()) {
    		fixNum = "1";
    	}
    	else {
    		fixNum = "0";
    	}
    	
    	numNuc = editNumNucText.getText();
    	
    	if (editSlidingWindowCheck.isSelected()) {
    		slidingWindow = "1";
    	}
    	else {
    		slidingWindow = "0";
    	}
    	
    	widthSW = editWidthSwText.getText();
    	jumpSW = editJumpSwText.getText();
    	
    	windowType = Integer.toString(editWindowTypeBox.getSelectedIndex());
    	
    	//doesn't really do anything right now
    	return true;
    }
       
    public boolean writeVsConfigFile (File file){
    	try {
    		BufferedWriter configOut = new BufferedWriter(new FileWriter(file));
    		configOut.write("# VariScan Config file created by VariScanGUI");configOut.newLine();
    		configOut.newLine();
    		configOut.write("StartPos = " + startPos);configOut.newLine();
    		configOut.write("EndPos = " + endPos);configOut.newLine();
    		configOut.write("RefPos = " + refPos);configOut.newLine();
    		configOut.write("BlockDataFile = " + bdfFile);configOut.newLine();
    		configOut.write("IndivNames = " + indivNames);configOut.newLine();
    		configOut.write("SeqChoice = " + seqChoice);configOut.newLine();
    		configOut.write("Outgroup = " + outgroup);configOut.newLine();
    		configOut.write("RefSeq = " + refSeq);configOut.newLine();
    		configOut.write("RunMode = " + runMode);configOut.newLine();
    		configOut.write("UseMuts = " + useMuts);configOut.newLine();
    		configOut.write("CompleteDeletion = " + completeDeletion);configOut.newLine();
    		configOut.write("FixNum = " + fixNum);configOut.newLine();
    		configOut.write("NumNuc = " + numNuc);configOut.newLine();
    		configOut.write("SlidingWindow = " + slidingWindow);configOut.newLine();
    		configOut.write("WidthSW = " + widthSW);configOut.newLine();
    		configOut.write("JumpSW = " + jumpSW);configOut.newLine();
    		configOut.write("WindowType = " + windowType);configOut.newLine();
    		configOut.write("UseLDSinglets = " + useLdSinglets);configOut.newLine();
    		
    		configOut.close();
    	}
    	catch (IOException ioe) {
    		writeFailed("the config file");
    		return false;
    	}
    	writeSuccessful("the config file");
    	return true;
    }
    
    public void setDefaultLwConfigValues () {
    	lwStatCol = "11";
    	lwRefCol = "3";
    	lwSubA = "6";
    	lwSubB = "9";
    	lwSubC = "12";
    	lwFilter = "D4.o";
    }
    
    public boolean setLwConfigValues () {
    	lwStatCol = lwStatColText.getText();
    	lwRefCol = lwRefColText.getText();
    	lwSubA = lwSubAText.getText();
    	lwSubB = lwSubBText.getText();
    	lwSubC = lwSubCText.getText();
    	lwFilter = lwFilterText.getText();

    	//doesn't really do anything right now
    	return true;
    }
    
    public boolean writeLwConfigFile (File file) {
    	try {
    		BufferedWriter configOut = new BufferedWriter(new FileWriter(file));
    		configOut.write("# LastWave Config file created by VariScanGUI");configOut.newLine();
    		configOut.newLine();
    		configOut.write("stat_col = " + lwStatCol);configOut.newLine();
    		configOut.write("ref_col = " + lwRefCol);configOut.newLine();
    		configOut.write("sub_a = " + lwSubA);configOut.newLine();
    		configOut.write("sub_b = " + lwSubB);configOut.newLine();
    		configOut.write("sub_c = " + lwSubC);configOut.newLine();
    		configOut.write("filter = " + lwFilter);configOut.newLine();
    		configOut.newLine();
    		configOut.write("lwpath = " + lwDir);configOut.newLine();
    		configOut.write("lwsourcedir = " + lwSourceDir);configOut.newLine();
    		configOut.write("lwbinary = " + lwBinFile);configOut.newLine();
    		
    		configOut.close();
    	}
    	catch (IOException ioe) {
    		writeFailed("the config file");
    		return false;	
    	}
    	writeSuccessful("the config file");
    	return true;
    }
    
    public void chainVstoLw () {
    	lwInputFile = vsOutputFile;
    	lwInputText.setText(vsOutputText.getText());
    	autosetLwOutputFile();
    }
    
    public void chainLwtoVis () {
    	visInputFile = lwOutputFile;
    	visInputText.setText(lwOutputText.getText());
    	autosetVisOutputFile();
    }
    
    public void autosetLwOutputFile () {
    	String name = lwInputFile.getName();
    	if (name.matches(".*\\..*$")) {
    		//file has a suffix
    		name = name.replaceFirst("\\..*$",".mra");
    	}
    	else {
    		name = name.concat(".mra");
    	}
    	lwOutputFile = new File(lwInputFile.getParent(),name);
    	lwOutputText.setText(lwOutputFile.getPath());
    }
    
    public void autosetVisOutputFile () {
    	String name = visInputFile.getName();
    	String ext;
    	
    	if (name.equals("")) {
    		//no input file defined yet
    		return;
    	}
    	
    	if (visType == 0) {
    		ext = ".bed";
    	}
    	else {
    		ext = ".xyplot";
    	}
    	
    	if (name.matches(".*\\..*$")) {
    		//file has a suffix
    		name = name.replaceFirst("\\..*$",ext);
    	}
    	else {
    		name = name.concat(ext);
    	}
    	visOutputFile = new File(visInputFile.getParent(),name);
    	visOutputText.setText(visOutputFile.getPath());
    }

    public boolean checkVsConfigConflicts () {
    	boolean val = true;
    	
    	val = checkRunMode31Conflict();
    	
    	return val;
    }
    
    public boolean checkRunMode31Conflict () {
    	boolean val = true;
    	if (runMode.equals("31") && slidingWindow.equals("0")) {
			JFrame frame = new JFrame();
			JOptionPane.showMessageDialog(frame,
				"The active VariScan config file uses RunMode 31 (LD analysis)\n" +  
				"without a sliding window analysis! Please change this in the\n"+
				"VariScan config file editor!",
				"VariScan config file conflict",
				JOptionPane.ERROR_MESSAGE);
			val = false;
    	}
    	
    	if (runMode.equals("31") && completeDeletion.equals("0")) {
			JFrame frame = new JFrame();
			JOptionPane.showMessageDialog(frame,
				"The active VariScan config file uses RunMode 31 (LD analysis)\n" +  
				"without complete deletion! Please change this in the VariScan \n" +
				"config file editor!",
				"VariScan config file conflict",
				JOptionPane.ERROR_MESSAGE);
			val = false;
    	}
		return val;
    }
    
    public boolean checkLwConfigConflicts () {
    	return true;
    }
    
    public boolean checkLwSettings () {
		if (!lwBinFile.exists() || !lwDir.exists() || !lwSourceDir.exists()) {
			JFrame frame = new JFrame();
			JOptionPane.showMessageDialog(frame,
				"Please make sure that the LastWave directory, LastWave\n" +  
				"source directory and LastWave binary are correctly set\n" +
				"under OPTIONS/SETTINGS",
				"Settings incomplete",
				JOptionPane.ERROR_MESSAGE);
			
			return false;
		}
		return true;
    }
    
    public boolean confirmOverwrite () {
    	JFrame frame = new JFrame();
    	int value = JOptionPane.showOptionDialog(frame,
    	"The file already exists. Overwrite?", "Confirm overwrite",
    	JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE, 
    	null, null, null);
    	if (value == JOptionPane.YES_OPTION) {
    		return true;
    	}
    	return false;
    }
    
    public void readFailed (String text) {
    	JFrame frame = new JFrame();
    	JOptionPane.showMessageDialog(frame,
    		"Unable to read " + text + ".\n" +
    		"Make sure you have permission to read the file.",
    		"Read file unsuccessful",
    		JOptionPane.ERROR_MESSAGE);
    }

    public void writeFailed (String text) {
    	JFrame frame = new JFrame();
    	JOptionPane.showMessageDialog(frame,
    		"Unable to save " + text + ".\n" +
    		"Make sure the file is not in use.",
    		"Save file unsuccessful",
    		JOptionPane.ERROR_MESSAGE);
    }
    
    public void writeSuccessful (String text) {
    	JFrame frame = new JFrame();
    	JOptionPane.showMessageDialog(frame,
    		"Successfully saved " + text + ".\n",
    		"Save file successful",
    		JOptionPane.INFORMATION_MESSAGE);
    }
    
    public void createFailed (String text) {
    	JFrame frame = new JFrame();
    	JOptionPane.showMessageDialog(frame,
    		"Unable to create " + text + "\n" +
    		"Make sure you have write permission in the corresponding directory.",
    		"Create file unsuccessful",
    		JOptionPane.ERROR_MESSAGE);
    }
    
    /**
     * Create the GUI and show it.  For thread safety,
     * this method should be invoked from the
     * event-dispatching thread.
     */
    private void createAndShowGUI() {
        //Make it look like the system
        try {
        	UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}
		
        //Create and set up the window.
        JFrame frame = new JFrame("VariScanGui");
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        //Create and set up the content pane.
        //VariScanGui demo = new VariScanGui();
        frame.setJMenuBar(createMenuBar());
        
        JPanel contentPane = new JPanel();
        contentPane.setLayout(new BoxLayout(contentPane, BoxLayout.PAGE_AXIS));
        contentPane.setOpaque(true);
        contentPane.add(createMainPanel());
        contentPane.add(createStatusPanel());
        
        frame.setContentPane(contentPane);
        
        //check for saved settings
		if (settingsFile.exists()) {
			readSettingsFile();
		}
		
		//check for recent runs
		if (recentFile.exists()) {
			readRecentFile();
		}
		
        //Display the window.
        frame.pack();
        frame.setVisible(true);
    }

    public static void main(String[] args) {
        //Schedule a job for the event-dispatching thread:
        //creating and showing this application's GUI.
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                VariScanGUI gui = new VariScanGUI();
                gui.createAndShowGUI();
            }
        });
    }
}
