#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <string>
#include <optional>

enum class MenuOption
{
    NewGame,
    Help,
    Exit
};

class Menu
{
public:
    Menu();
    
    // Main menu functions
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    MenuOption getSelectedOption() const;
    bool isOptionChosen() const;
    void reset();
    
    // Help screen functions
    void drawHelpScreen(sf::RenderWindow& window);
    void setShowHelp(bool showHelp);
    bool isShowingHelp() const;
    
    // Sound mute functions - centralized here
    void toggleMute();
    void drawMuteIcon(sf::RenderWindow& window);
    bool isMuted() const;
    void setMuted(bool muted);
    sf::FloatRect getMuteIconBounds(sf::RenderWindow& window) const;
    
private:
    // Menu options
    std::vector<std::string> m_options = {"NEW GAME", "HELP", "EXIT"};
    std::vector<sf::FloatRect> m_buttonBounds;
    int m_selectedIndex = 0;
    bool m_optionChosen = false;

    // Help screen state
    bool m_showHelp = false;

    // Mute state - Menu only owns the state, GameController handles sound updates
    bool m_isMuted = false;
};