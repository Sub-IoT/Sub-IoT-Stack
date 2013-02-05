using System;
using System.IO.Ports;
using System.Threading;
using System.Collections.Generic;
using System.Windows.Forms;

namespace Dash7Logger
{
	public class GatewayInterface: IDisposable
	{
		private SerialPort serialPort;
		private string comPort = string.Empty;
		private List<byte> data;
		private int dataLength = 0;
		private bool keyboardEmulation = false;		
		
		public GatewayInterface ()
		{
			data = new List<byte>();
		}
		
		public bool OpenConnectionToCOM (string comPort)
		{
			bool connectedToCom = false;
			while (!connectedToCom)
			{
				try
				{
					serialPort = new SerialPort(comPort, 115200, Parity.None, 8, StopBits.Two);						
					//serialPort.DataReceived += HandleSerialPortDataReceived;
					serialPort.Open ();
					Console.WriteLine("Port open");
					
					Thread readThread = new Thread(new ThreadStart(ReadThread));
        			readThread.Start();
					
					return true;
				}
				catch (Exception)
				{
					Console.WriteLine("COM Port {0} can not be opened", comPort);	
					Console.WriteLine("Retry (y/n)");
					
					string input = Console.ReadLine();
					
					if (input != "y")
						return false;
				}	
			}
			
			return false;
		}

		public void EnableKeyboardEmulation ()
		{
			keyboardEmulation = true;
		}
		
		private void HandleSerialPortDataReceived (object sender, SerialDataReceivedEventArgs e)
		{
			
			SerialPort sp = (SerialPort)sender;
			
			int bytesToRead = sp.BytesToRead;
			byte[] data = new byte[bytesToRead];
			
			sp.Read (data, 0, bytesToRead);
			
			// Parse Data
			this.ParseData(data);			
		}
		
		private void ReadThread()
        {
			try 
			{
	            while(true)
	            {
	                Thread.Sleep(200);
	                if(serialPort.BytesToRead > 0)
	                {
	                    HandleSerialPortDataReceived(serialPort, null);
	                }
	            }
			} catch (Exception ex)
			{
				Console.WriteLine("Serial port connection broken");	
			}
        }
		
		
		//DD00 0D 0204 02 444154414C454E4754 20 303331 0BF10473
		//DD00 23 0204 02 525353 20 1F32FF21E540 2DAAAA1DBBBBBBB100A0020000000B1DAAAA1DBBBBBBB1C7FF0BF208B8
		// 6 bit header, datalength message, 4 bit footer
		public void ParseData (byte[] bytes)
		{
			System.Text.UTF8Encoding enc = new System.Text.UTF8Encoding();
			try 
			{
				
				data.AddRange(bytes);	
				
				while(data.Count > 1 && (data[0] != 0xDD && data[0] != 0x00))
					data.RemoveAt(0);

                while (data.Count >= 20 && data[0] == 0x00)
                {
                    if (data[1] == 0x02 && data[19] == 0x0D)
                    {
                        List<byte> packet = data.GetRange(2, 17);

                        List<byte> fixedNodeId = packet.GetRange(0, 8);
                        List<byte> mobileNodeId = packet.GetRange(8, 8);
                        int rss = packet[16] - 256;


                        Console.WriteLine(string.Format("{0} - {1}: {2} dBm", OSLCore.Converter.ConvertByteArrayToHexString(fixedNodeId.ToArray()), OSLCore.Converter.ConvertByteArrayToHexString(mobileNodeId.ToArray()), rss));

                        data.RemoveRange(0, 20);
                    }
                    else
                    {
                        data.Clear();
                    }
                }
				
				while (data.Count > 3 && data[0] == 0xDD)
	    		{
					byte type = data[1];
	        		dataLength = data[2];
		        	if (dataLength > 0 && dataLength + 3 <= data.Count)
		        	{
		            	//byte nr = data[2];
		                //data.RemoveRange(0, 6);
					
						switch (type)
						{
						case 0x00:	
						{
							List<byte> packet = data.GetRange(3, dataLength);
							Console.WriteLine(string.Format("{0} DATA: {1}", DateTime.Now.ToString ("HH:mm:ss.fff"), OSLCore.Converter.ConvertByteArrayToHexString(packet.ToArray())));
							int length = (int) packet[0];
							double txreip = -40 + (((int) packet[1])  * 0.5);
							byte subnet = packet[2];
							byte framectrl = packet[3];
							Console.WriteLine(string.Format(" - Length:   {0} bytes", length));
							Console.WriteLine(string.Format(" - TX ERIP:  {0:0.00} dBm", txreip));
							Console.WriteLine(string.Format(" - Subnet:   {0:X}",  subnet));
							Console.WriteLine(string.Format(" - FrameCtl: {0:X}",  framectrl));
							
							if ((framectrl & (1 << 7)) != 0) Console.WriteLine("    - Listen");
							if ((framectrl & (1 << 6)) != 0) Console.WriteLine("    - Data Link Layer Security");
							if ((framectrl & (1 << 5)) != 0) Console.WriteLine("    - Enable Addressing");
							if ((framectrl & (1 << 4)) != 0) Console.WriteLine("    - Frame Continuity");
							if ((framectrl & (1 << 3)) != 0) Console.WriteLine("    - Use CRC32");
							if ((framectrl & (1 << 2)) != 0) Console.WriteLine("    - Not Mode 2");							
							if ((framectrl & 3) == 0) Console.WriteLine("    - Dialog Frame (M2NP)");				
							if ((framectrl & 3) == 1) Console.WriteLine("    - Dialog NACK (M2NP)");			
							if ((framectrl & 3) == 2) Console.WriteLine("    - Stream Frame (M2DP)");		
							if ((framectrl & 3) == 3) Console.WriteLine("    - RFU");
							
							int i=4;
							//TODO: check real headers
							byte addr_dialog = packet[i];
							byte addr_flags = packet[i+1];
							i+=2;
							
							Console.WriteLine(string.Format(" - Addressing - Dialog ID:   {0:X}",  addr_dialog));
							Console.WriteLine(string.Format(" - Addressing - Flags:       {0:X}",  addr_flags));
							
							if (((addr_flags >> 6) & 3) == 0) Console.WriteLine("    - Unicast");
							if (((addr_flags >> 6) & 3)  == 1) Console.WriteLine("    - Broadcast");
							if (((addr_flags >> 6) & 3)  == 2) Console.WriteLine("    - Anycast");
							if (((addr_flags >> 6) & 3)  == 3) Console.WriteLine("    - Multicast");
							if ((addr_flags & (1 << 5)) != 0) Console.WriteLine("    - Virtual ID");
							if ((addr_flags & (1 << 4)) != 0) Console.WriteLine("    - Network Layer Security");
							if ((addr_flags & 15) != 0) Console.WriteLine(String.Format ("    - Application Flag: {0:X}", addr_flags & 15));
							
							List<byte> deviceId;
							int deviceIdLength = ((addr_flags & (1 << 5)) != 0) ? 2 : 8;
							deviceId = packet.GetRange(i, deviceIdLength);
							i += deviceIdLength;
							
							Console.WriteLine(string.Format(" - Device ID: {0}", OSLCore.Converter.ConvertByteArrayToHexString(deviceId.ToArray())));
							
							byte dunno = packet[i++];
							byte isfId = packet[i];
							byte isfOffset = packet[i+1];
							byte isfDataLength = packet[i+2];
							i+=3;
							
							List<byte> payload = packet.GetRange(i, isfDataLength);
							
							Console.WriteLine(string.Format(" - Payload - Don't know:  {0:X}",  dunno));
							Console.WriteLine(string.Format(" - Payload - ISF ID:      {0:X}",  isfId));
							Console.WriteLine(string.Format(" - Payload - ISF Offset:  {0}",  isfOffset));
							Console.WriteLine(string.Format(" - Payload - Data Length: {0}",  isfDataLength));
							Console.WriteLine(string.Format(" - Payload - Data:        {0}",  OSLCore.Converter.ConvertByteArrayToHexString(payload.ToArray())));
							
							if (keyboardEmulation && isfId == 0x00 && isfDataLength == 1)
							{
								switch (payload[0])
								{						
								case 0x01:
									SendKeys.SendWait("{LEFT}");
									break;
								case 0x02:
									SendKeys.SendWait(" ");
									break;
								case 0x03:
									SendKeys.SendWait("{RIGHT}");
									break;
								}
							}
							
							break;
						}
						case 0x01:	
						{
							Console.WriteLine(string.Format("{0} {1}",  DateTime.Now.ToString ("HH:mm:ss.fff"), enc.GetString(data.GetRange(3, dataLength).ToArray())));
							break;
						}
						case 0x02:	// RX res
						{
							List<byte> rx_res = data.GetRange(3, 5);
							Console.WriteLine(string.Format("RSSI: {0} dBm", (SByte) rx_res[1]));
							Console.WriteLine(string.Format("EIRP: {0} dBm", rx_res[2]));
							Console.WriteLine(string.Format("LQI:  {0}", rx_res[3]));
							Console.WriteLine(string.Format("Length:  {0}", rx_res[4]));
							List<byte> packet = data.GetRange(8, rx_res[4]);
							Console.WriteLine(string.Format("{0} DATA: {1}", DateTime.Now.ToString ("HH:mm:ss.fff"), OSLCore.Converter.ConvertByteArrayToHexString(packet.ToArray())));
							int length = (int) packet[0];
							double txreip = -40 + (((int) packet[1])  * 0.5);
							byte subnet = packet[2];
							byte framectrl = packet[3];
							Console.WriteLine(string.Format(" - Length:   {0} bytes", length));
							Console.WriteLine(string.Format(" - TX ERIP:  {0:0.00} dBm", txreip));
							Console.WriteLine(string.Format(" - Subnet:   {0:X}",  subnet));
							Console.WriteLine(string.Format(" - FrameCtl: {0:X}",  framectrl));
							
							if ((framectrl & (1 << 7)) != 0) Console.WriteLine("    - Listen");
							if ((framectrl & (1 << 6)) != 0) Console.WriteLine("    - Data Link Layer Security");
							if ((framectrl & (1 << 5)) != 0) Console.WriteLine("    - Enable Addressing");
							if ((framectrl & (1 << 4)) != 0) Console.WriteLine("    - Frame Continuity");
							if ((framectrl & (1 << 3)) != 0) Console.WriteLine("    - Use CRC32");
							if ((framectrl & (1 << 2)) != 0) Console.WriteLine("    - Not Mode 2");							
							if ((framectrl & 3) == 0) Console.WriteLine("    - Dialog Frame (M2NP)");				
							if ((framectrl & 3) == 1) Console.WriteLine("    - Dialog NACK (M2NP)");			
							if ((framectrl & 3) == 2) Console.WriteLine("    - Stream Frame (M2DP)");		
							if ((framectrl & 3) == 3) Console.WriteLine("    - RFU");
							
							int i=4;
							//TODO: check real headers
							byte addr_dialog = packet[i];
							byte addr_flags = packet[i+1];
							i+=2;
							
							Console.WriteLine(string.Format(" - Addressing - Dialog ID:   {0:X}",  addr_dialog));
							Console.WriteLine(string.Format(" - Addressing - Flags:       {0:X}",  addr_flags));
							
							if (((addr_flags >> 6) & 3) == 0) Console.WriteLine("    - Unicast");
							if (((addr_flags >> 6) & 3)  == 1) Console.WriteLine("    - Broadcast");
							if (((addr_flags >> 6) & 3)  == 2) Console.WriteLine("    - Anycast");
							if (((addr_flags >> 6) & 3)  == 3) Console.WriteLine("    - Multicast");
							if ((addr_flags & (1 << 5)) != 0) Console.WriteLine("    - Virtual ID");
							if ((addr_flags & (1 << 4)) != 0) Console.WriteLine("    - Network Layer Security");
							if ((addr_flags & 15) != 0) Console.WriteLine(String.Format ("    - Application Flag: {0:X}", addr_flags & 15));
							
							List<byte> deviceId;
							int deviceIdLength = ((addr_flags & (1 << 5)) != 0) ? 2 : 8;
							deviceId = packet.GetRange(i, deviceIdLength);
							i += deviceIdLength;
							
							Console.WriteLine(string.Format(" - Device ID: {0}", OSLCore.Converter.ConvertByteArrayToHexString(deviceId.ToArray())));
							
							byte dunno = packet[i++];
							byte isfId = packet[i];
							byte isfOffset = packet[i+1]; 
							byte isfDataLength = packet[i+2];
							i+=3;
							
							List<byte> payload = packet.GetRange(i, isfDataLength);
							
							Console.WriteLine(string.Format(" - Payload - Don't know:  {0:X}",  dunno));
							Console.WriteLine(string.Format(" - Payload - ISF ID:      {0:X}",  isfId));
							Console.WriteLine(string.Format(" - Payload - ISF Offset:  {0}",  isfOffset));
							Console.WriteLine(string.Format(" - Payload - Data Length: {0}",  isfDataLength));
							Console.WriteLine(string.Format(" - Payload - Data:        {0}",  OSLCore.Converter.ConvertByteArrayToHexString(payload.ToArray())));
							i+= isfDataLength;

							List<byte> crc = packet.GetRange(i, 2);
							Console.WriteLine(string.Format(" - CRC:  {0}",  OSLCore.Converter.ConvertByteArrayToHexString(crc.ToArray())));
							break;
						}
						case 0xFF: // command
							{
							List<byte> command = data.GetRange(3, dataLength);
							switch (command[0])
							{
							case 0xB0: // request date time
								DateTime time = DateTime.Now;
								List<byte> response = new List<byte>();
								response.AddRange(BitConverter.GetBytes((short)(time.Year)));
								response.AddRange(BitConverter.GetBytes((char) (time.Month)));
								response.AddRange(BitConverter.GetBytes((char) (time.Day)));


								break;
							}
							break;
							}
						}						
						data.RemoveRange (0, 3 + dataLength);
			        }
					else
					{
						break;
					}
			    }
				
			} catch (Exception ex)
			{
				Console.WriteLine(ex.Message);	
			}
			
		    //data.Clear();	
		}
		
		#region IDisposable implementation
		public void Dispose ()
		{
			if (this.serialPort != null)
			{
				this.serialPort.Close();
				this.serialPort.Dispose();
			}
		}
		#endregion
	}
}