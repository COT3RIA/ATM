#pragma once
#include <juce_core/juce_core.h>
#include <atomic>
#include <map>

// Interface language (English / Russian). The default follows the language of the system,
// the choice made with the LANG key is stored in the per-user settings file.
namespace lang
{
    inline std::atomic<int>& state() { static std::atomic<int> s { -1 }; return s; }      // -1 = not initialised, 0 = EN, 1 = RU

    inline juce::File settingsFile()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("COTERIA").getChildFile ("ATM.settings");
    }

    inline void init()
    {
        if (state().load() >= 0)
            return;

        int v = 0;
        juce::PropertiesFile pf (settingsFile(), juce::PropertiesFile::Options());
        const auto saved = pf.getValue ("language", {});
        if (saved == "ru")      v = 1;
        else if (saved == "en") v = 0;
        else
        {
            const auto sys = (juce::SystemStats::getUserLanguage() + " " + juce::SystemStats::getDisplayLanguage()).toLowerCase();
            v = sys.trim().startsWith ("ru") || sys.contains (" ru") ? 1 : 0;
        }
        state() = v;
    }

    inline bool isRu() { init(); return state().load() == 1; }

    inline void set (bool russian)
    {
        init();
        state() = russian ? 1 : 0;
        settingsFile().getParentDirectory().createDirectory();
        juce::PropertiesFile pf (settingsFile(), juce::PropertiesFile::Options());
        pf.setValue ("language", russian ? "ru" : "en");
        pf.saveIfNeeded();
    }

    inline const std::map<juce::String, juce::String>& table()
    {
        static const std::map<juce::String, juce::String> t = []
        {
            std::map<juce::String, juce::String> m;
            auto add = [&] (const char* en, const char* ru) { m[juce::String::fromUTF8 (en)] = juce::String::fromUTF8 (ru); };

            // keys on the screen
            add ("OPEN", "ОТКРЫТЬ");        add ("PLAY", "ИГРАТЬ");        add ("STOP", "СТОП");
            add ("HISTORY", "ИСТОРИЯ");     add ("SETTINGS", "НАСТРОЙКИ"); add ("SOLO", "СОЛО");
            add ("RESET", "СБРОС");         add ("NEXT", "ДАЛЕЕ");         add ("BACK", "НАЗАД");
            add ("THEME", "ТЕМА");          add ("UPDATE!", "ОБНОВИТЬ!");  add ("LANG: EN", "ЯЗЫК: RU");
            add ("LANG: RU", "ЯЗЫК: RU");   add ("START: 0", "СТАРТ: 0");  add ("START: AUDIO", "СТАРТ: АУДИО");

            // tooltips
            add ("Open an audio file", "Открыть аудиофайл");
            add ("Play / stop the preview", "Прослушать / остановить");
            add ("History of your MIDI files", "История ваших MIDI-файлов");
            add ("Dark / light theme of the panels", "Тёмная / светлая тема панелей");
            add ("Show / hide the Advanced Settings", "Показать / скрыть расширенные настройки");
            add ("COTERIA on Telegram", "COTERIA в Telegram");
            add ("Solo instrument preset (one guitar, piano...): fewer extra notes. Sets Note sensitivity 0.60 and Split sensitivity 0.30",
                 "Пресет для одного инструмента (гитара, пианино...): меньше лишних нот. Ставит чувствительность нот 0.60 и разделение нот 0.30");
            add ("Next / previous page of keys", "Следующая / предыдущая страница кнопок");
            add ("Reset the settings to defaults", "Сбросить настройки по умолчанию");
            add ("MIDI position: keep the position the notes have in the audio / start the first note at 0 of the grid",
                 "Положение MIDI: как в аудио / первая нота в нуле сетки");
            add ("Insert an audio file (MP3 / WAV / FLAC), or drop it on the window",
                 "Вставьте аудиофайл (MP3 / WAV / FLAC) или перетащите его в окно");
            add ("Interface language: English / Russian", "Язык интерфейса: English / Русский");
            add ("Tell me when a new version of ATM is available", "Сообщать о выходе новой версии ATM");
            add ("Click to type a value", "Нажмите, чтобы ввести значение");
            add ("Detected tempo and key. Click the key to switch between the two most likely keys",
                 "Найденный темп и тональность. Нажмите на тональность, чтобы переключить на второй по вероятности вариант");
            add ("Choose the folder for generated MIDI files", "Выберите папку для MIDI-файлов");
            add ("Choose an audio file", "Выберите аудиофайл");

            // receipt
            add ("RECEIPT", "ЧЕК");  add ("TRACK", "ТРЕК");
            add ("Drag the receipt out as a picture", "Перетащите чек как картинку");

            // screen
            add ("Drop audio here", "Перетащите аудио сюда");
            add ("Converting...", "Конвертация...");
            add ("drag into your DAW", "перетащите в DAW");

            // history
            add ("History", "История");
            add ("Drag a file into your DAW. Double-click shows it in the folder.", "Перетащите файл в DAW. Двойной клик покажет его в папке.");
            add ("No MIDI files yet", "MIDI-файлов пока нет");
            add ("Open folder", "Открыть папку");  add ("Delete", "Удалить");  add ("Close", "Закрыть");

            // settings panel
            add ("Advanced Settings", "Доп. настройки");
            add ("Set MIDI folder...", "Папка MIDI...");
            add ("Note sensitivity", "Чувствит. нот");   add ("Split sensitivity", "Разделение нот");
            add ("Min note length", "Мин. длина ноты");  add ("Min frequency", "Мин. частота");
            add ("Max frequency", "Макс. частота");      add ("Note-off hangover", "Затухание ноты");
            add (" ms", " мс");  add (" Hz", " Гц");  add (" frames", " кадр.");  add ("Off", "Выкл");
            add ("Infer Onsets", "Атаки");  add ("Smoothing", "Сглаж.");  add ("Updates", "Обновл.");
            add ("Play", "Играть");  add ("Stop", "Стоп");  add ("Reset to defaults", "Сбросить");
            add ("KEY", "ТОН");  add (" min", " мин");  add (" maj", " маж");

            // receipt
            add ("FILE", "ФАЙЛ");  add ("TEMPO", "ТЕМП");  add ("NOTES", "НОТЫ");  add ("LENGTH", "ДЛИНА");
            add ("SENS", "ЧУВСТВ.");  add ("SURE", "УВЕРЕН.");  add ("DATE", "ДАТА");  add ("THANK YOU!", "СПАСИБО!");
            add ("Drag the receipt to share it. Click to close.", "Перетащите чек, чтобы поделиться. Нажмите, чтобы закрыть.");
            add ("Click to enlarge the receipt", "Нажмите, чтобы увеличить чек");

            // processor messages
            add ("Drop an audio file (MP3 / WAV / FLAC) onto the window", "Перетащите аудиофайл (MP3 / WAV / FLAC) в окно");
            add ("Re-analysing...", "Повторный анализ...");
            add ("The audio is too short or silent", "Аудио слишком короткое или тихое");
            add ("Loading ", "Загрузка ");  add ("Analysing ", "Анализ ");  add ("Could not read ", "Не удалось прочитать ");
            add ("could not write the MIDI file, check the MIDI folder", "не удалось записать MIDI, проверьте папку");
            add (" BPM auto", " BPM авто");  add ("from 0", "с нуля");
            add ("New version ", "Новая версия ");
            add (" is available - open the COTERIA channel", " доступна: откройте канал COTERIA");
            return m;
        }();
        return t;
    }

    inline juce::String tr (const juce::String& en)
    {
        if (! isRu())
            return en;
        const auto it = table().find (en);
        return it != table().end() ? it->second : en;
    }

    // "21 notes" / "21 нота" with the Russian plural forms
    inline juce::String notes (int n)
    {
        if (! isRu())
            return juce::String (n) + " notes";
        const int a = n % 100, b = n % 10;
        const char* w = (a >= 11 && a <= 14) ? "нот" : b == 1 ? "нота" : (b >= 2 && b <= 4) ? "ноты" : "нот";
        return juce::String (n) + " " + juce::String::fromUTF8 (w);
    }
}
