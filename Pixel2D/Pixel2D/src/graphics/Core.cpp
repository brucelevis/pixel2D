#include "Core.hpp"
#include "..\utils\Themes.hpp"
#include "..\utils\Utility.hpp"
#include <imgui-SFML.h>
#include <imguidock.h>
#include <SFML\Window\Event.hpp>

namespace px
{
	Core::Core() : m_window(sf::VideoMode(1400, 900), "Pixel2D", sf::Style::Close), m_circle(25.f), m_movementSpeed(250.f)
	{
		initialize();
	}

	Core::~Core()
	{
		ImGui::SFML::Shutdown();
	}

	void Core::initialize()
	{
		//Window
		m_window.setPosition({ 125, 75 });
		m_window.setVerticalSyncEnabled(true);

		//Circle
		m_circle.setOrigin(m_circle.getRadius(), m_circle.getRadius()); 
		m_circle.setFillColor(sf::Color::Green);
		m_currentCirclePosition = sf::Vector2f(670.f, 233.f);
		m_previousCirclePosition = m_currentCirclePosition;

		//Render texture
		m_sceneTexture.create(m_window.getSize().x, m_window.getSize().y);

		//Timestep
		m_timestep.setStep(1.0 / 60.0);
		m_timestep.setMaxAccumulation(0.25); //Maximum time processed

		//GUI
		DarkWhiteTheme(true, 0.9f);
		ImGui::SFML::Init(m_window);

		//Scene
		m_scene = std::make_unique<Scene>(m_sceneTexture);
	}

	void Core::run()
	{
		sf::Clock guiClock;
		while (m_window.isOpen())
		{
			processEvents();
			update();
			ImGui::SFML::Update(m_window, guiClock.restart());
			updateGUI();

			std::string infoTitle;
			infoTitle += "Pixel2D || Fps: " + std::to_string(m_fps.getFps());
			m_window.setTitle(infoTitle.c_str());
			m_window.clear();
			ImGui::SFML::Render(m_window);
			m_window.display();
		}
	}

	void Core::render()
	{
		m_sceneTexture.clear(sf::Color::Black);
		m_scene->updateSystems(m_timestep.getStep());
		m_sceneTexture.draw(m_circle);
		m_sceneTexture.display();
	}

	void Core::processEvents()
	{
		sf::Event event;
		while (m_window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				m_window.close();
		}
	}

	void Core::update()
	{
		m_fps.update();
		m_timestep.addFrame(); //Add frame each cycle
		while (m_timestep.isUpdateRequired()) //If there are unprocessed timesteps
		{
			m_previousCirclePosition = m_currentCirclePosition;
			float dt = m_timestep.getStepAsFloat();

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) // move up
				m_currentCirclePosition.y += m_movementSpeed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) // move down
				m_currentCirclePosition.y -= m_movementSpeed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) // move left
				m_currentCirclePosition.x -= m_movementSpeed * dt;
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) // move right
				m_currentCirclePosition.x += m_movementSpeed * dt;
		}

		float interpolationAlpha = m_timestep.getInterpolationAlphaAsFloat();

		//Update objects
		m_circle.setPosition(utils::linearInterpolation(m_previousCirclePosition, m_currentCirclePosition, interpolationAlpha));
	}

	void Core::updateGUI()
	{
		//Docking system
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		const ImGuiWindowFlags flags = (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoTitleBar);
		const float oldWindowRounding = ImGui::GetStyle().WindowRounding; ImGui::GetStyle().WindowRounding = 0;
		const bool visible = ImGui::Begin("Docking system", NULL, ImVec2(0, 0), 1.0f, flags);
		ImGui::GetStyle().WindowRounding = oldWindowRounding;
		ImGui::SetWindowPos(ImVec2(0, 10));

		if (visible)
		{
			ImGui::BeginDockspace();

			ImGui::SetNextDock(ImGuiDockSlot_Left);
			if (ImGui::BeginDock("Scene"))
			{
				ImVec2 size = ImGui::GetContentRegionAvail();
				unsigned int width = m_sceneTexture.getSize().x;
				unsigned int height = m_sceneTexture.getSize().y;

				if (width != size.x || height != size.y)
					m_sceneTexture.create((unsigned int)size.x, (unsigned int)size.y);

				render();

				//Draw the image/texture, filling the whole dock window
				ImGui::Image(m_sceneTexture.getTexture());
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Bottom);
			if (ImGui::BeginDock("Debug"))
			{
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Inspector"))
			{
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Left);
			if (ImGui::BeginDock("Hierarchy"))
			{
				ImGui::BeginChild("Entities");
				ImGui::EndChild();
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Bottom);
			if (ImGui::BeginDock("Log"))
			{
				//gameLog.Draw();
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Console"))
			{
				//gameConsole.Draw();
			}
			ImGui::EndDock();

			ImGui::SetNextDock(ImGuiDockSlot_Tab);
			if (ImGui::BeginDock("Assets"))
			{
			}
			ImGui::EndDock();

			ImGui::EndDockspace();
		}
		ImGui::End();
	}
}