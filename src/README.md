# 🌍 Antipode: DayDreamNight  

### "See the other side of your world."

A Qt-based global video social prototype — developed for **XJCO2811 User Interfaces (The Process)**.

---

## 📖 Overview

**Antipode: DayDreamNight (DDN)** is a Qt-based video social prototype inspired by the geographical concept of **antipodes** — two points on Earth that are diametrically opposite.  

The app allows users to see "what's happening on the other side of the world" in real time — *your night is their day*.

This project follows **three Agile development sprints**, including prototyping, implementation, evaluation, and final showcase video.

## Contents

- [Core Concept](#-core-concept)
- [Feature Highlights](#-feature-highlights)
- [App Structure](#-app-structure--five-navigation-pages)
- [Sprint Releases](#-sprint-releases)
- [Project Architecture](#-project-architecture)
- [Running Locally](#-running-locally)
- [Troubleshooting](#-troubleshooting)
- [Code Organization](#-code-organization)
- [License & Team](#-license)

---

## 🧭 Core Concept

- 🌎 **Antipode**: Automatically calculate the user's antipodal point  
- ☀️ **DayDreamNight**: Visual metaphor of day–night contrast  
- 🎥 **Authenticity**: Once-per-day genuine short video  
- 🌐 **Internationalisation**: English/Chinese localisation, time formats  
- ♿ **Accessibility**: High-contrast mode, colour-blind safe palette, large touch areas  

## ✨ Feature Highlights

- **Real media pipeline** – scans the `videos/` directory, generates thumbnails when available, and streams via a shared Qt Multimedia player.  
- **Full localisation loop** – `LanguageManager` surfaces English/Chinese copies across every page, and `NarrationManager` mirrors the setting for TTS prompts.  
- **Accessibility surfaces** – global subtitles toggle, high-contrast palettes, narration cues, and larger hit targets on critical controls.  
- **Procedural social data** – Friends and Profile pages fabricate believable posts, avatars, captions, and stats so the UI always feels populated.  
- **Stacked navigation shell** – `home/Antipode.cpp` wires all five tabs into one `QStackedWidget` with shared settings dialogs and state.  

---

# 🚀 Sprint Releases

- **Sprint 3 (v3.0)** – [Release tag](https://github.com/Jiacheeng-Li/The-Process-Group-12/releases/tag/v3.0-sprint3)  
- **Sprint 2 (v2.0)** – [Release tag](https://github.com/Jiacheeng-Li/The-Process-Group-12/releases/tag/v2.0-sprint2)  
- **Sprint 1 (v1.0)** – [Release tag](https://github.com/Jiacheeng-Li/The-Process-Group-12/releases/tag/v1.0-sprint1)  

---

# 📱 App Structure — Five Navigation Pages

Bottom navigation bar layout (stacked pages defined in `home/Antipode.cpp`):

```
🏠 Home     👥 Friends     ⏺ Record     💬 Chat     👤 Profile
```

---

## 🏠 Home

The Home page is the landing experience and shows the antipode storytelling card plus the active playback session.

**Implemented behavior**
- **Earth-themed background**: swaps between `earth.png` and `earth1.png` to mimic day/night on opposite hemispheres  
- **Real-drop card**: displays the current poster’s avatar ring, name, city, landmark, timestamp, and “late” indicator generated from metadata  
- **Embedded video player**: mounts the custom `ThePlayer` widget to play clips discovered in the `videos/` folder (with subtitles and playback controls)  
- **Accessibility toggles**: integrates `SubtitleController` and `NarrationManager` so users can enable subtitles, switch subtitle language, or hear spoken prompts  
- **Settings dialog**: the top-right button opens an in-app settings sheet to tweak language, subtitle availability, and narration preferences  

---

## 👥 Friends

Friends is a vertically scrolling feed that fabricates believable friend activity from local demo assets.

**Implemented behavior**
- **Scrollable timeline**: `QScrollArea + QVBoxLayout` fills the page with stacked `FriendItem` cards and suppresses scrollbars for a cleaner look  
- **Procedural posts**: pulls thumbnails from the local `videos/` directory, pairs them with predefined names/hashtags/caption templates (English & Chinese copies), and timestamps each entry  
- **Avatar and preview loading**: attempts to find `.png` thumbnails alongside each video and profile photos under `friends/avatar/` to populate the cards  
- **Comment affordance**: each card exposes a comment action via the `commentRequested` signal, which the main app can hook into for future flows  
- **Theme + locale aware**: cards automatically adopt day/night or high-contrast palettes and reflect the active `LanguageManager` setting  

---

## ⏺ Record

Record is a camera UI prototype that simulates a short-video capture workflow without talking to real hardware.

**Implemented behavior**
- **Phone-frame canvas**: a faux handset (`phoneFrame`) hosts the main capture viewport plus a floating front-camera preview window  
- **Recording control**: the large red shutter toggles between recording/not recording, flashes via a `QTimer`, and triggers narration prompts for each state change  
- **Transport & camera controls**: pause/resume and camera-switch buttons appear around the shutter to suggest full recording controls  
- **Draft entry point**: the Drafts pill button summons a `PopupPanel` listing saved drafts, demonstrating how users would revisit unfinished clips  
- **Language + narration**: `LanguageManager` swaps button labels, while `NarrationManager` announces “recording started/stopped” in the chosen language  
- **Responsive layout**: `resizeEvent` recalculates the handset geometry so the prototype remains proportionally correct on different window sizes  

---

## 💬 Chat

Chat is a dual-column messaging experience with a selectable conversation list and bubble-based transcript.

**Implemented behavior**
- **Conversation list**: a `QListWidget` on the left shows preset contacts with status text; two localized datasets (English/Chinese) live in code for testing language toggles  
- **Bubble transcript**: the right pane builds incoming/outgoing bubbles (`bubbleIncoming` / `bubbleOutgoing`) and auto-scrolls to the latest message  
- **Message composer**: the input box + Send button (and Enter shortcut) append messages to the current thread and clear the field  
- **Contact switching**: selecting a different contact calls `switchContact`, updates the header labels, and rebuilds the transcript from the stored history  
- **High-contrast theme**: `setHighContrastMode` swaps the full stylesheet to a yellow-on-black palette for accessibility demos  
- **Narration + localization**: `LanguageManager` updates headings/placeholders/hints; `NarrationManager` speaks out contact switches and send confirmations in the active language  

---

## 👤 Profile

Profile showcases the user’s identity, stats, filter tabs, and video grid mock-ups.

**Implemented behavior**
- **Gradient avatar ring**: recreates an Instagram-style ring plus circular crop; avatars are looked up from `friends/avatar/` using the provided username  
- **Identity fields**: `setUserInfo` accepts username, display name, bio, and avatar path; missing values fall back to computed defaults (e.g., “@username”)  
- **Dynamic stats**: following/follower/like counts are pseudo-randomized per username and formatted into raw numbers, K, or M for realism  
- **Filter bar**: Grid / Drafts / Tagged buttons (with `filterKey` properties) retheme their labels on language changes and drive which placeholder content is shown  
- **Theme modes**: day/night and high-contrast modes are supported, including alternate SVG assets for follow/share buttons when necessary  
- **Localization + narration**: `localizedProfileCopy` provides English/Chinese text variants and `NarrationManager` keeps narration in sync with the chosen language  

---

# 🏗️ Project Architecture

## 🗂️ File Architecture

```
docs/
  Ethics and evidence of feedback/
  sprint1/
  sprint2/
  sprint3/
src/
  capture/
  chat/
  friends/
  home/
  player/
  profile/
  shared/
  README.md
  the.pro
videos/
.gitignore
README.md
```

## 📁 Directory Structure

| Path | Purpose |
|------|---------|
| `<project_root>/src/the.pro` | Qt project descriptor opened in Qt Creator |
| `<project_root>/src/home/` | Navigation shell, entry point, shared dialogs |
| `<project_root>/src/capture/` | Record, publish, popup panel components |
| `<project_root>/src/friends/` | Friend feed widgets, friend item, avatars |
| `<project_root>/src/chat/` | Chat list + conversation UI |
| `<project_root>/src/profile/` | Profile page, stats, filters |
| `<project_root>/src/player/` | `ThePlayer`, transport buttons, media helpers |
| `<project_root>/src/shared/` | Language manager, narration manager |
| `<project_root>/videos/` | Local sample videos passed as runtime argument |

## 🧩 Technical Stack

| Module | Qt Components |
|--------|---------------|
| Video playback | `QMediaPlayer`, `QVideoWidget` |
| Video recording | `QCamera`, `QVideoProbe` |
| Earth model | `QOpenGLWidget` |
| UI layout | `QStackedWidget`, `QScrollArea` |
| Messaging | `QTcpSocket`, `QListView` |
| Animations | `QPropertyAnimation` |
| Text-to-Speech | `QTextToSpeech` (for accessibility) |

## 🔧 Module Architecture

### Core Modules

1. **home/Antipode.cpp** - Application entry point
   - Initializes QApplication
   - Sets up main window and navigation
   - Manages page switching

2. **player/** - Video playback engine
   - `the_player`: Core media player wrapper
   - `the_button`: Custom control buttons
   - Handles video loading, playback, and controls

3. **capture/** - Video recording & publishing
   - `recordpage`: Camera interface and recording
   - `publishpage`: Video preview and metadata input
   - `popuppanel`: UI popups for settings/tags

4. **friends/** - Social feed
   - `friendspage`: Timeline view
   - `frienditem`: Individual post widget
   - `video_viewer`: Full-screen video viewer

5. **chat/** - Messaging system
   - `chat_page`: Chat interface with message list
   - Real-time message display

6. **profile/** - User profile
   - `profile_page`: Profile display and editing
   - Video archive management

7. **shared/** - Utility modules
   - `language_manager`: i18n support (English/Chinese)
   - `narration_manager`: Voice narration for accessibility

---

# ⚙️ Running Locally

## Prerequisites

- **Qt 5.15.2** or compatible version
- **Qt Multimedia** module support
- **Qt TextToSpeech** module (for accessibility features)
- C++11 compiler support

## Step-by-Step Instructions

### 1. Install Qt

Download and install **Qt 5.15.2** from [qt.io](https://www.qt.io/download)  
**Important**: Ensure the following components are installed:
- Qt 5.15.2 (or compatible)
- Qt Multimedia
- Qt Multimedia Widgets
- Qt TextToSpeech

### 2. Clone the Repository

```bash
git clone <repository-url>
cd <project_root>/src
```

### 3. Open Project in Qt Creator

1. Launch **Qt Creator**
2. Go to `File > Open File or Project...`
3. Navigate to `src/the.pro`
4. Click **Open**

### 4. Configure Build Settings

1. Go to `Projects` (left sidebar)
2. Select your build kit (e.g., Desktop Qt 5.15.2 MSVC2019 64-bit)
3. Ensure the build directory is set correctly

### 5. Set Video Folder Path (Required)

The application requires a video folder path as a command-line argument.

**In Qt Creator:**
1. Go to `Projects > Run Settings`
2. Under "Command line arguments", enter:
   ```
   <absolute/path/to/videos>
   ```
   Example:
   ```
   D:\path\to\project\videos
   ```
   Or on Linux/Mac:
   ```
   /path/to/videos
   ```

**Alternative: Run from Command Line**

```bash
# Windows (PowerShell)
cd src
qmake the.pro
nmake  # or mingw32-make for MinGW
.\release\the.exe D:\path\to\videos

# Linux/Mac
cd src
qmake the.pro
make
./the /path/to/videos
```

### 6. Build and Run

1. Click the **▶️ Run** button (or press `Ctrl+R`)
2. The application should launch with the video feed

## ✅ Demo Checklist

- Launch Home, verify the Real-drop card shows city/location metadata and subtitles toggle works.  
- Switch to Friends, scroll to confirm procedural posts render avatars/thumbnails.  
- Open Record, tap the shutter to hear narration feedback and see the pulse animation.  
- Send a message in Chat, then switch contacts to confirm localized status text updates.  
- Visit Profile, toggle between Grid/Drafts/Tagged and flip the app to high-contrast mode to confirm recolored icons.  

---

## 🐛 Troubleshooting

### Common Issues

**Issue**: "Cannot find video files"  
**Solution**: Ensure the video folder path is correct and contains video files

**Issue**: "Qt Multimedia not found"  
**Solution**: Reinstall Qt with Multimedia module enabled

**Issue**: "Camera not working"  
**Solution**: Check camera permissions in system settings

**Issue**: Build errors  
**Solution**: Ensure Qt 5.15.2 is properly installed and selected as the kit

---

# 📊 Code Organization

## File Naming Convention

- **Page files**: `*page.cpp/h` (e.g., `chat_page.cpp`, `profile_page.cpp`)
- **Component files**: `the_*.cpp/h` (e.g., `the_player.cpp`, `the_button.cpp`)
- **Manager files**: `*_manager.cpp/h` (e.g., `language_manager.cpp`)

## Key Design Patterns

- **Page-based navigation**: Each main feature is a separate page class
- **Shared utilities**: Common functionality in `shared/` directory
- **Resource management**: Icons and assets via `.qrc` files
- **Separation of concerns**: UI, logic, and data handling separated

---

# 📜 License

Academic use only — for XJCO2811 coursework. 
All video and image assets in the prototype have been cleared and authorized for use.
All interviews complied with ethics requirements; 
participants were informed and provided written consent.


---

# 👥 Team

Developed by Group 12 for XJCO2811 User Interfaces course.

Team: Jiacheng Li; Yuanxi Zhong; Na Cao; Ziyi Yan; Xintong Li.
