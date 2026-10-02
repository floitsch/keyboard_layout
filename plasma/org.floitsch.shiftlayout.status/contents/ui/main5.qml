import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
Item {
    id: root
    readonly property bool vertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property int thickness: Math.max(1, Plasmoid.configuration.thickness)
    property string mode: "unavailable"
    readonly property string description: mode === "meta" ? i18n("Meta locked")
        : mode === "scroll" ? i18n("Scrolling locked")
        : mode === "idle" ? i18n("Mouse mode inactive")
        : mode === "waiting" ? i18n("Waiting for mouse") : i18n("Mouse service unavailable")

    Layout.minimumWidth: vertical ? 1 : thickness
    Layout.preferredWidth: vertical ? 32 : thickness
    Layout.maximumWidth: vertical ? Infinity : thickness
    Layout.minimumHeight: vertical ? thickness : 1
    Layout.preferredHeight: vertical ? thickness : 32
    Layout.maximumHeight: vertical ? thickness : Infinity
    Layout.fillWidth: vertical
    Layout.fillHeight: !vertical
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground
    Plasmoid.toolTipMainText: root.description
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    PlasmaCore.DataSource {
        id: statusSource
        engine: "executable"
        // Periodic engine polling is clamped to one second. Each fresh source
        // executes immediately; disconnect it when the short read completes.
        readonly property string command: "if read -r pid mode 2>/dev/null < /run/shift-layout-mouse/state && case $pid in ''|*[!0-9]*) false;; *) test -d /proc/$pid;; esac; then printf '%s' \"$mode\"; else printf unavailable; fi"
        readonly property string instanceId: String(Math.random())
        property int requestNumber: 0
        property bool pending: false
        connectedSources: []
        interval: 0
        function readStatus() {
            if (pending) return;
            pending = true;
            connectSource(command + " # " + instanceId + "/" + (++requestNumber));
        }
        onNewData: {
            disconnectSource(sourceName);
            pending = false;
            var value = String(data["stdout"] || "").trim();
            root.mode = ["meta", "scroll", "idle", "waiting"].indexOf(value) >= 0
                ? value : "unavailable";
        }
    }

    Timer {
        interval: 100
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: statusSource.readStatus()
    }

    Rectangle {
        anchors.fill: parent
        color: root.mode === "meta" ? "#ff9800"
            : root.mode === "scroll" ? "#2196f3"
            : root.mode === "idle" ? "#606060" : "#e53935"
        opacity: root.mode === "idle" ? 0.3 : 1
        Accessible.name: root.description
        Accessible.role: Accessible.Indicator
    }
}
