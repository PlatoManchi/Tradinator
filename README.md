> [!CAUTION]
> This app is for educational purposes only and does not offer financial or investment advice. Use of any suggestions is at your own risk. The developer is not responsible for any investment decisions or losses.

# Intro
Tradinator is an app I built to put my learnings about patterns and strategies into pratice.  

> [!NOTE]
> Depending on your hardware, first time setup might take 15min or more since downloading all the historical data and processing them into a sqlite database is slow and takes time. Bottle neck writting into sqlite. 


# Setting Working Folder
When running the app first time, start up screen is shown. 
Working folder is the folder where app will for all its operations. Downloaded raw candle data is stored at NSE/Raw folder and the processed sqlite database is stored at Data/Tradinator.db.

Other settings are self explinatory and have explinations on what they do. These settings can be changes latter in settings menu.

> [!NOTE]
> If the text isn't crips, press alt+enter to go full screen and again press alt+enter to exit full screen. This will reset the DPI settings in ImGui to make font more crisp.

![Start Up screen](https://github.com/PlatoManchi/Tradinator/blob/50c947a7d72c7e8e311e8d24135dff815c0917a1/Documentation/Screenshot%202025-06-05%20115725.png)



# Setting Markets
Once working folder is set, you will have to download the security list from nse market. Simply click on check box or visit [https://nsearchives.nseindia.com/content/equities/EQUITY_L.csv](https://nsearchives.nseindia.com/content/equities/EQUITY_L.csv) and place the csv file (working folder)/NSE/EQUITY_L.csv

# Setting Visibility
Set the visibility of patterns and strategies when opening a security. Use this to show only revelant information or to declutter the security window.
These are accessable in settings or directly in file menu.
![Setting patterns and strategies visibility in security window](https://github.com/PlatoManchi/Tradinator/blob/ca6c10caf1dd85cb8d68a022dceff06b8e61bf15/Documentation/Screenshot%202025-06-05%20115754.png)

# Dashboard
- On top of dashboard there is search bar where you can look for any security.
- You can pin or unpin a security for quick access. These pinned securities can be accessed on left side.
- Middle part is work in progress, will show trends and securities with most movement etc as dashboard.
- Tradinator will automatically analyze all the candle data for all securities and generate news. This news is on right size and is arranged by date. This lets you see hilights of all the securities at one place instead of having to open each and everyone to check them.

> [!WARNING]
> All the data is downloaded from a external website called upstox. And a API call has to be made for each security candle data. If too many API calls are made in short time, server might reject the API calls with error 1015. If this happens the candle data for those securities won't be updated.
>
> The raw downloaded data can be accessed at working folder/NSE/Raw

![Dashboard](https://github.com/PlatoManchi/Tradinator/blob/ca6c10caf1dd85cb8d68a022dceff06b8e61bf15/Documentation/Screenshot%202025-06-05%20120239.png)


# Security Window
Here you can apply indicators, change the window size or other parameters for the indicators and change the colors for better visibility.
P in candle chart represents that there is a pattern at that point
S in candle chart represents that there is a strategy detected at that point. Green line means its long opportunity and red line means its short opportunity.
> [!CAUTION]
> Currently the app cannot detect support and resistance. So there are errors in pattern recognization as well as short and long opportinuties detections.
> Cannot detect consolidation break out or break down so look for them manually before doing long or short. This is the reason why strategies aren't perfect and need human eye before commiting to them.

![Security Window Example](https://github.com/PlatoManchi/Tradinator/blob/ca6c10caf1dd85cb8d68a022dceff06b8e61bf15/Documentation/Screenshot%202025-06-05%20120644.png)


