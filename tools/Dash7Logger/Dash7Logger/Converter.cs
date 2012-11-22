using System;

namespace OSLCore
{
    public static class Converter
    {
        public static string ConvertByteArrayToHexString(byte[] array)
        {
            if (array == null)
                return string.Empty;

            return BitConverter.ToString(array).Replace("-", string.Empty);
        }

        public static byte[] ConvertHexStringToByteArray(string hexString)
        {
			try
			{
	            hexString = hexString.Replace("-", string.Empty);
	            hexString = hexString.Replace(":", string.Empty);
	            int numChars = hexString.Length;
	            byte[] byteArray = new byte[numChars / 2];
	
	            for (int i = 0; i < numChars; i += 2)
	            {
	                byteArray[i / 2] = Convert.ToByte(hexString.Substring(i, 2), 16);
	            }

            	return byteArray;
			}
			catch (Exception ex)
			{
				return null;
			}
        }
    }
}
