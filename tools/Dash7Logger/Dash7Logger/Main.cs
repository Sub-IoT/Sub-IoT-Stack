using System;
using System.IO.Ports;

namespace Dash7Logger
{
	class MainClass
	{		
		static SerialPort serialPort = null;
		
		public static void Main (string[] args)
		{
			string comPort = string.Empty;	
			bool emulateKeyboard = false;
			bool argParsingOK = true;
			
			try
			{
				
				for (int a=0; a < args.Length; a++)
				{
					switch (args[a])
					{
					case "-c":
						if (++a < args.Length)
						{
							comPort = args[a];
							Console.WriteLine("Selected COM Port: {0}", comPort);
						} 
						else {
							argParsingOK = false;
						}
						break; 
					case "-k":
						if (++a < args.Length)
						{
							if (args[a] == "1")
							{
								emulateKeyboard = true;
								Console.WriteLine("Emulate keyboard enabled");
							} else 
							{								
								Console.WriteLine("Emulate keyboard disabled");
							}
						} 
						else {
							argParsingOK = false;
						}
						break; 
					default:
						argParsingOK = false;
						break;
					}
				}
				
				if (!argParsingOK)
				{
					Console.WriteLine("Error parsing arguments");
					Console.WriteLine("-c COMport");
					Console.WriteLine("-k 1 enables keyboard emulation");
				}
				
				if (comPort == string.Empty)
				{
                    Console.WriteLine("Give COM Port: ");
                    foreach (string port in SerialPort.GetPortNames())
                    {
                        Console.WriteLine(port);
                    }
                    comPort = Console.ReadLine();
					Console.WriteLine("{0} will be used", comPort);				
				}	
				
				
				GatewayInterface gateway = new GatewayInterface();
				
				// Open Serial Port COMport				
				gateway.OpenConnectionToCOM(comPort);
				if (emulateKeyboard)
					gateway.EnableKeyboardEmulation();
				
				
				
				Console.WriteLine("Options: 'x' to exit application");
				string readline = string.Empty;
				while (readline != "x")
				{
					readline = Console.ReadLine();
				}
				
				gateway.Dispose();
				
				
			}
			catch (Exception ex)
			{
				Console.WriteLine("Exception: " + ex.Message);	
			}
		}
	}
}
