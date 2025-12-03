
# 🌍 Antipode: DayDreamNight  
### “See the other side of your world.”
A Qt-based global video social prototype — developed for **XJCO2811 User Interfaces (The Process)**.

---

## 📖 Overview

**Antipode: DayDreamNight (DDN)** is a Qt-based video social prototype inspired by the geographical concept of **antipodes** — two points on Earth that are diametrically opposite.  
The app allows users to see “what’s happening on the other side of the world” in real time — *your night is their day*.

This project follows **three Agile development sprints**, including prototyping, implementation, evaluation, and final showcase video.

---

## 🧭 Core Concept

- 🌎 **Antipode**: Automatically calculate the user’s antipodal point  
- ☀️ **DayDreamNight**: Visual metaphor of day–night contrast  
- 🎥 **Authenticity**: Once-per-day genuine short video  
- 🌐 **Internationalisation**: English/Chinese localisation, time formats  
- ♿ **Accessibility**: High-contrast mode, colour-blind safe palette, large touch areas  

---

# 📱 App Structure — Five Navigation Pages

Bottom navigation bar layout:

```
🏠 Home     👥 Friends     ⏺ Record     💬 Chat     👤 Profile
```

---

## 🏠 Home
Displays recommended videos from the user’s **antipodal location**.

**Features**
- Auto antipode calculation  
- Vertical swipe video feed  
- ❤️ Like  
- 💬 Comment  
- ↗ Share  
- Swipe-left → user profile  

---

## 👥 Friends
Timeline-based view showing content from followed users and friends.

**Features**
- Time-ordered video feed  
- Tap thumbnail → full video  
- Jump to other user's profile  

---

## ⏺ Record
Users can record and publish one short video per day.

**Features**
- Front/back camera switch  
- 10-second recording  
- Preview  
- Add tags & description  
- Upload → sync to Profile  

---

## 💬 Chat
Direct messaging between friends.

**Features**
- Chat list  
- Text & emoji  
- Real-time updates  
- Back navigation  

---

## 👤 Profile
Manage personal information and showcase pinned videos.

**Features**
- Edit avatar, nickname, bio  
- Automatic “Today’s video” section  
- Pin-to-top videos  
- View full video archive  

---

# 🧩 Technical Stack

| Module | Qt Components |
|--------|---------------|
| Video playback | `QMediaPlayer`, `QVideoWidget` |
| Video recording | `QCamera`, `QVideoProbe` |
| Earth model | `QOpenGLWidget` |
| UI layout | `QStackedWidget`, `QScrollArea` |
| Messaging | `QTcpSocket`, `QListView` |
| Animations | `QPropertyAnimation` |

---

# 📁 Project Folder Structure

```
/ (repo root)
├── README.md
├── 2811_cw3-master-release-lowres/
├── docs/
│   ├── sprint1/
│   ├── sprint2/
│   └── sprint3/
├── src/
│   ├── the.pro
│   ├── capture/
│   ├── chat/
│   ├── friends/
│   ├── home/
│   ├── player/
│   ├── profile/
│   ├── sources/
│   └── videos/
├── videos/
└── .gitignore
```

---

# ⚙️ How to Run

### 1. Install Qt  
Recommended version: **Qt 5.15.2** (with Multimedia support)

### 2. Clone the repository
```
git clone https://github.com/Jiacheeng-Li/The-Process-Group-12
```

### 3. Open in Qt Creator
```
Open > .pro or CMakeLists.txt
```

### 4. Set video folder argument  
In "Project > Run Settings":

```
<absolute/path/to/videos>
```

### 5. Run  
Press ▶️ Run in Qt Creator.

---

# 👥 Team Roles

| Member | Responsibilities |
|--------|------------------|
| **JC** | UI/UX design, visual style guide, project management, GitHub structure, video editing |
| **CN** | Home & Player implementation |
| **ZY** | Frontend implementation, Chat & Profile, UI integration |
| **XT** | Documentation, requirements, use cases, meeting notes, ethics |
| **YX** | Record & Friends modules, accessibility design |

---

# 🔀 Version Control Workflow

### Branches
```
main

sprint1-JC-uiux
sprint1-CN-player
sprint1-ZY-frontend
sprint1-YX-capture
sprint1-XT-docs

sprint1-prototype

sprint2-JC-uiux
sprint2-CN-player
sprint2-ZY-frontend-player
sprint2-YX-capture
sprint2-XT-docs

sprint2-integration
sprint2-prototype
```

### Tags
```
v1.0-sprint1
v2.0-sprint2
v3.0-sprint3
```

---

# 🧪 Iteration Plan

| Sprint | Goal |
|--------|------|
| Sprint 1 | UI prototyping, basic framework, MVP pages |
| Sprint 2 | UI implementation, interactions, i18n & accessibility |
| Sprint 3 | Accessibility, internationalisation, and a unified UI experience |
| Final | Showcase video + full report |

---

# 📜 License
Academic use only — for XJCO2811 coursework.
