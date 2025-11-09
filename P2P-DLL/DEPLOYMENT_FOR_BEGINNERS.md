# P2P Network DLL - Beginner's Deployment Guide

**Last Updated**: November 9, 2025  
**Difficulty**: Beginner-Friendly  
**Time Required**: 15-30 minutes  
**Status**: ✅ Production Ready

---

## 📖 What is This Guide For?

This guide will help you add **P2P (Peer-to-Peer) networking** to your Ragnarok Online client. Don't worry if you're not technical - we'll explain everything step by step!

---

## 🤔 What Does P2P Do? (In Simple Terms)

### Without P2P (Normal):
```
Player 1 ──→ Server ──→ Player 2
Player 2 ──→ Server ──→ Player 1
```
Every message goes through the server. The server does all the work.

### With P2P (Enhanced):
```
Player 1 ←──→ Player 2 (Direct Connection)
```
Players can talk directly to each other. The server only handles important stuff like NPCs and items.

### Why Would I Want This?

**Benefits:**
- ✅ **Faster gameplay** - Less lag when chatting or moving near other players
- ✅ **Better performance** - Server has less work to do, so it runs smoother
- ✅ **More players** - Server can handle more people at once

**Important:** P2P is **completely optional**! If it doesn't work or you don't want it, the game automatically uses the normal server connection. Nothing breaks!

---

## ✅ Prerequisites Checklist

Before you start, make sure you have:

- [ ] **Windows 10 or Windows 11** (64-bit)
- [ ] **Ragnarok Online client** installed on your computer
- [ ] **NEMO Patcher** (included in this package at `patcher/WARP-p2p-client/Nemo/`)
- [ ] **P2P Network DLL** already built (the file `p2p_network.dll` exists in `patcher/WARP-p2p-client/P2P-DLL/build/bin/Release/`)
- [ ] **Administrator rights** on your computer (to copy files and run patcher)

**Don't have the DLL built yet?** See [BUILD_GUIDE.md](BUILD_GUIDE.md) for instructions, or ask your server administrator for the pre-built files.

---

## 📁 Step 1: Find Your Files

### 1.1 Locate the P2P DLL Files

Open File Explorer and navigate to:
```
d:\RO\patcher\WARP-p2p-client\P2P-DLL\build\bin\Release\
```

You should see these files:
- ✅ `p2p_network.dll` (568 KB) - **This is the main file!**
- ✅ `libcrypto-3-x64.dll` (5.3 MB) - Encryption library
- ✅ `libssl-3-x64.dll` (871 KB) - Security library
- ✅ `spdlog.dll` (285 KB) - Logging library
- ✅ `fmt.dll` (120 KB) - Formatting library
- ✅ Other DLL files (brotli, gtest, etc.)

**Can't find these files?** The DLL hasn't been built yet. See [BUILD_GUIDE.md](BUILD_GUIDE.md).

### 1.2 Locate Your RO Client Folder

Find where your Ragnarok Online game is installed. Common locations:
- `C:\Program Files (x86)\Ragnarok Online\`
- `C:\Games\Ragnarok Online\`
- `D:\RO\` (if you installed it on D: drive)

**Look for:** The folder containing `Ragnarok.exe` or your client executable.

---

## 📋 Step 2: Copy DLL Files to RO Client

### 2.1 Copy All DLL Files

1. **Select all DLL files** from the Release folder (Step 1.1)
2. **Copy them** (Ctrl+C or right-click → Copy)
3. **Navigate to your RO client folder** (Step 1.2)
4. **Paste the files** (Ctrl+V or right-click → Paste)

**Important:** Copy ALL the DLL files, not just `p2p_network.dll`! The main DLL needs the other files to work.

### 2.2 Copy Configuration File

1. Go to `d:\RO\patcher\WARP-p2p-client\P2P-DLL\config\`
2. Copy `p2p_config.json`
3. Paste it into your RO client folder (same place as the DLLs)

**Your RO client folder should now have:**
- ✅ All the DLL files
- ✅ `p2p_config.json`
- ✅ Your original RO client files (Ragnarok.exe, etc.)

---

## 🔧 Step 3: Configure P2P Settings

### 3.1 Open the Configuration File

1. In your RO client folder, find `p2p_config.json`
2. Right-click it → **Open with** → **Notepad** (or any text editor)

### 3.2 Understand the Settings (Simple Explanation)

You'll see a file with settings like this:

```json
{
  "coordinator": {
    "rest_api_url": "http://localhost:8001/api/v1",
    "websocket_url": "ws://localhost:8001/api/v1/signaling/ws",
    ...
  },
  "p2p": {
    "enabled": true,
    ...
  }
}
```

**Key Settings You Might Want to Change:**

| Setting | What It Does | Default | Should I Change? |
|---------|--------------|---------|------------------|
| `p2p.enabled` | Turn P2P on/off | `true` | **No** - Leave it on |
| `coordinator.rest_api_url` | Server address | `localhost:8001` | **Yes** - Ask your server admin |
| `coordinator.websocket_url` | WebSocket address | `localhost:8001` | **Yes** - Ask your server admin |
| `logging.level` | How much logging | `info` | **No** - Leave as is |
| `zones.p2p_enabled_zones` | Which maps use P2P | Prontera, Geffen, etc. | **Maybe** - See below |

### 3.3 Update Server URLs (Important!)

**If you're playing on a server (not localhost):**

1. **Ask your server administrator** for the coordinator URLs
2. Replace `localhost:8001` with the actual server address

Example:
```json
"rest_api_url": "http://your-server.com:8001/api/v1",
"websocket_url": "ws://your-server.com:8001/api/v1/signaling/ws",
```

**If you're testing locally:** Leave it as `localhost:8001`

### 3.4 Choose P2P-Enabled Zones (Optional)

The `zones.p2p_enabled_zones` setting controls which maps use P2P:

```json
"p2p_enabled_zones": [
  "prontera",    // Prontera city
  "geffen",      // Geffen city
  "payon",       // Payon city
  "morocc",      // Morroc city
  "alberta",     // Alberta city
  "aldebaran",   // Al De Baran city
  "izlude"       // Izlude city
]
```

**Recommendation:** Start with just busy cities (prontera, geffen). You can add more later!

### 3.5 Save the File

1. **Save** the file (Ctrl+S or File → Save)
2. **Close** the text editor

---

## 🎨 Step 4: Patch Your RO Client with NEMO

### 4.1 What is NEMO?

NEMO is a tool that modifies your RO client executable to add new features (like P2P support). It's safe and commonly used in the RO community.

### 4.2 Open NEMO Patcher

1. Navigate to `d:\RO\patcher\WARP-p2p-client\Nemo\`
2. Double-click `Nemo.exe` to run it
3. **If Windows asks:** Click "Yes" to allow (needs admin rights)

### 4.3 Load Your Client

1. In NEMO, click **"Load Client"** button
2. Browse to your RO client folder
3. Select your client executable (e.g., `Ragnarok.exe` or `2023-11-01Ragexe.exe`)
4. Click **Open**

### 4.4 Select P2P Patches

In the NEMO patch list, find and **check** these patches:

**Required Patches:**
- ✅ **Load P2P DLL** - Loads the p2p_network.dll file
- ✅ **Enable P2P Networking** - Activates P2P features

**Recommended Patches** (optional but helpful):
- ✅ **Disable Packet Encryption** - Makes P2P work better (if your server allows)
- ✅ **Increase Zoom Out** - Better gameplay experience
- ✅ **Enable /who Command** - See who's online

**Don't check patches you don't understand!** Stick to the required ones for now.

### 4.5 Apply Patches

1. Click the **"Apply Patches"** button
2. Choose where to save the patched client:
   - **Recommended:** Save as `Ragnarok_P2P.exe` in your RO client folder
   - This keeps your original client untouched!
3. Wait for NEMO to finish (usually 10-30 seconds)
4. You should see **"Patching completed successfully!"**

### 4.6 Close NEMO

Click **Exit** or close the NEMO window.

---

## 🚀 Step 5: Test Your P2P Client

### 5.1 Launch the Patched Client

1. Go to your RO client folder
2. Double-click `Ragnarok_P2P.exe` (or whatever you named it)
3. **If Windows asks:** Click "Yes" to allow

### 5.2 Check if P2P is Working

**Method 1: Check the Log File**

1. After launching the client, look for `p2p_dll.log` in your RO client folder
2. Open it with Notepad
3. Look for lines like:
   ```
   [info] P2P Network DLL initialized successfully
   [info] Connected to coordinator server
   [info] P2P enabled for zone: prontera
   ```

**Method 2: Watch for Connection Messages**

When you enter a P2P-enabled zone (like Prontera), you might see:
- Slightly faster loading
- Smoother player movements
- Less lag when many players are nearby

**Method 3: Check with Server Admin**

Ask your server administrator if they can see your P2P connection in the coordinator logs.

---

## ❌ Step 6: What If Something Goes Wrong?

### Common Problems and Solutions

#### Problem 1: "DLL not found" error

**Cause:** The DLL files aren't in the right place.

**Solution:**
1. Make sure ALL DLL files are in the same folder as your RO client executable
2. Check that you copied `p2p_network.dll` AND all dependency DLLs
3. Try copying the files again

#### Problem 2: Client crashes on startup

**Cause:** Incompatible patches or missing dependencies.

**Solution:**
1. Use your original client (without P2P) to make sure the game works
2. Re-patch with NEMO, but ONLY check the "Load P2P DLL" patch
3. Make sure you have Visual C++ Redistributable installed:
   - Download from: https://aka.ms/vs/17/release/vc_redist.x64.exe
   - Install it and restart your computer

#### Problem 3: P2P doesn't seem to work

**Cause:** Configuration issue or coordinator server not running.

**Solution:**
1. Check `p2p_config.json` - are the server URLs correct?
2. Ask your server admin if the coordinator server is running
3. Check `p2p_dll.log` for error messages
4. **Don't worry!** The game will automatically use normal server connection if P2P fails

#### Problem 4: "Cannot connect to coordinator"

**Cause:** Wrong server URL or coordinator server is down.

**Solution:**
1. Verify the URLs in `p2p_config.json` with your server admin
2. Make sure your firewall isn't blocking the connection
3. **The game will still work!** It just won't use P2P

---

## 🔄 How to Disable/Remove P2P

### Temporary Disable (Keep Files)

**Option 1:** Edit `p2p_config.json`:
```json
"p2p": {
  "enabled": false,   ← Change true to false
  ...
}
```

**Option 2:** Use your original client executable instead of the patched one.

### Permanent Removal

1. Delete all the DLL files you copied (p2p_network.dll, libcrypto-3-x64.dll, etc.)
2. Delete `p2p_config.json`
3. Use your original client executable

**Your game will work perfectly fine without P2P!**

---

## ❓ Frequently Asked Questions (FAQ)

### Q1: Is P2P safe? Will I get banned?

**A:** Yes, it's safe! P2P is a legitimate networking technology. However:
- Only use it if your server administrator approves it
- Don't use it on official servers (they don't support it)
- Private servers need to have the coordinator service running

### Q2: Will P2P make my game faster?

**A:** In crowded zones (like Prontera), yes! You'll notice:
- Less lag when many players are nearby
- Smoother chat and movement
- Better overall performance

In empty zones or solo play, you won't notice much difference.

### Q3: Do I need to open ports on my router?

**A:** Usually no! P2P uses STUN servers to work through most routers automatically. If you have a very strict firewall, you might need to allow the RO client through it.

### Q4: What if other players don't have P2P?

**A:** No problem! The system is smart:
- If both players have P2P: They connect directly
- If one player doesn't have P2P: They use normal server connection
- Everything works seamlessly!

### Q5: Can I use P2P on multiple computers?

**A:** Yes! Just repeat these steps on each computer. Make sure they all use the same coordinator server URLs.

### Q6: How much bandwidth does P2P use?

**A:** Very little! Usually 50-500 KB/s depending on how many players are nearby. It's less than watching a YouTube video.

### Q7: What if I update my RO client?

**A:** You'll need to re-patch it with NEMO. The DLL files and config can stay the same.

### Q8: Where can I get help?

**A:** 
- Check the log file: `p2p_dll.log` in your RO client folder
- Ask your server administrator
- Read the technical docs: [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)
- Check the troubleshooting section below

---

## 🛠️ Troubleshooting Checklist

If P2P isn't working, go through this checklist:

- [ ] All DLL files are in the RO client folder
- [ ] `p2p_config.json` is in the RO client folder
- [ ] Server URLs in config are correct (ask your admin!)
- [ ] `p2p.enabled` is set to `true` in config
- [ ] Client was patched with NEMO successfully
- [ ] Using the patched client executable (not the original)
- [ ] Coordinator server is running (ask your admin!)
- [ ] Firewall allows the RO client
- [ ] Visual C++ Redistributable is installed
- [ ] Check `p2p_dll.log` for error messages

**Still not working?** That's okay! The game will use normal server connection automatically. P2P is optional!

---

## 📚 Next Steps

### For Regular Players:

✅ **You're done!** Just play the game normally. P2P works automatically in the background.

### For Advanced Users:

- Read [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) for technical details
- Learn about configuration options in [API_REFERENCE.md](API_REFERENCE.md)
- Understand WebRTC in [WEBRTC_GUIDE.md](WEBRTC_GUIDE.md)

### For Server Administrators:

- Set up the coordinator server (see DEPLOYMENT_GUIDE.md)
- Configure zones and security settings
- Monitor P2P connections and performance

---

## 📞 Getting Help

**If you're stuck:**

1. **Check the log file** (`p2p_dll.log`) - it often tells you what's wrong
2. **Ask your server administrator** - they can check if the coordinator is working
3. **Use the original client** - if P2P doesn't work, the game still works fine!

**Remember:** P2P is optional! If it doesn't work, don't worry - you can still play normally.

---

## ✅ Summary

**What you did:**
1. ✅ Copied DLL files to RO client folder
2. ✅ Copied and configured `p2p_config.json`
3. ✅ Patched your client with NEMO
4. ✅ Tested the P2P client

**What happens now:**
- When you enter P2P-enabled zones, you'll connect directly to other players
- If P2P doesn't work, the game automatically uses normal server connection
- Everything is transparent - just play and enjoy!

**Congratulations!** You've successfully set up P2P networking for Ragnarok Online! 🎉

