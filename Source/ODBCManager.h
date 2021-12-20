/*
  ==============================================================================

    ODBCManager.h
    Created: 20 Dec 2021 4:54:42pm
    Author:  DPR

  ==============================================================================
*/

#pragma once
///<summary>
/// Class to assist with creation and removal of ODBC DSN entries
///</summary>
static class ODBCManager
{
    private const string ODBC_INI_REG_PATH = "SOFTWARE\\ODBC\\ODBC.INI\\";
    private const string ODBCINST_INI_REG_PATH = "SOFTWARE\\ODBC\\ODBCINST.INI\\";

    /// <summary>
    /// Creates a new DSN entry with the specified values. If the DSN exists, the values are updated.
    /// </summary>
    /// <param name="dsnName">Name of the DSN for use by client applications</param>
    /// <param name="description">Description of the DSN that appears in the ODBC control panel applet</param>
    /// <param name="server">Network name or IP address of database server</param>
    /// <param name="driverName">Name of the driver to use</param>
    /// <param name="trustedConnection">True to use NT authentication, false to require applications to supply username/password in the connection string</param>
    /// <param name="database">Name of the datbase to connect to</param>
    public static void CreateDSN(string dsnName, string description, string server, string driverName, bool trustedConnection, string database)
    {
        // Lookup driver path from driver name
        var driverKey = Registry.LocalMachine.CreateSubKey(ODBCINST_INI_REG_PATH + driverName);
        if (driverKey == null) throw new Exception(string.Format("ODBC Registry key for driver '{0}' does not exist", driverName));
        string driverPath = driverKey.GetValue("Driver").ToString();

        // Add value to odbc data sources
        var datasourcesKey = Registry.LocalMachine.CreateSubKey(ODBC_INI_REG_PATH + "ODBC Data Sources");
        if (datasourcesKey == null) throw new Exception("ODBC Registry key for datasources does not exist");
        datasourcesKey.SetValue(dsnName, driverName);

        // Create new key in odbc.ini with dsn name and add values
        var dsnKey = Registry.LocalMachine.CreateSubKey(ODBC_INI_REG_PATH + dsnName);
        if (dsnKey == null) throw new Exception("ODBC Registry key for DSN was not created");
        dsnKey.SetValue("Database", database);
        dsnKey.SetValue("Description", description);
        dsnKey.SetValue("Driver", driverPath);
        dsnKey.SetValue("LastUser", Environment.UserName);
        dsnKey.SetValue("Server", server);
        dsnKey.SetValue("Database", database);
        dsnKey.SetValue("Trusted_Connection", trustedConnection ? "Yes" : "No");
    }

    /// <summary>
    /// Removes a DSN entry
    /// </summary>
    /// <param name="dsnName">Name of the DSN to remove.</param>
    public static void RemoveDSN(string dsnName)
    {
        // Remove DSN key
        Registry.LocalMachine.DeleteSubKeyTree(ODBC_INI_REG_PATH + dsnName);

        // Remove DSN name from values list in ODBC Data Sources key
        var datasourcesKey = Registry.LocalMachine.CreateSubKey(ODBC_INI_REG_PATH + "ODBC Data Sources");
        if (datasourcesKey == null) throw new Exception("ODBC Registry key for datasources does not exist");
        datasourcesKey.DeleteValue(dsnName);
    }

    ///<summary>
    /// Checks the registry to see if a DSN exists with the specified name
    ///</summary>
    ///<param name="dsnName"></param>
    ///<returns></returns>
    public static bool DSNExists(string dsnName)
    {
        var driversKey = Registry.LocalMachine.CreateSubKey(ODBCINST_INI_REG_PATH + "ODBC Drivers");
        if (driversKey == null) throw new Exception("ODBC Registry key for drivers does not exist");

        return driversKey.GetValue(dsnName) != null;
    }

    ///<summary>
    /// Returns an array of driver names installed on the system
    ///</summary>
    ///<returns></returns>
    public static string[] GetInstalledDrivers()
    {
        var driversKey = Registry.LocalMachine.CreateSubKey(ODBCINST_INI_REG_PATH + "ODBC Drivers");
        if (driversKey == null) throw new Exception("ODBC Registry key for drivers does not exist");

        var driverNames = driversKey.GetValueNames();

        var ret = new List<string>();

        foreach(var driverName in driverNames)
        {
            if (driverName != "(Default)")
            {
                ret.Add(driverName);
            }
        }

        return ret.ToArray();
    }
}