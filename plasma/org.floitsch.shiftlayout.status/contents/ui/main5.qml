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

    // The service binary's watch mode blocks until the published mode differs
    // from the one passed in, so each result starts the next watch. A data
    // source keeps a map entry for every source name it has seen, and reusing a
    // name can return the engine's cached result; use a fresh data source with
    // a unique name per watch instead.
    readonly property string watchCommand: "/usr/lib/shift_layout/mouse-meta-toggle --watch "
    readonly property string instanceId: String(Math.random())
    property string reported: "none"
    property int requestNumber: 0

    function watch() {
        watchComponent.createObject(root).connectSource(watchCommand + reported
            + " # " + instanceId + "/" + (++requestNumber));
    }

    function handleStatus(data) {
        var value = String(data["stdout"] || "").trim();
        mode = ["meta", "scroll", "idle", "waiting"].indexOf(value) >= 0
            ? value : "unavailable";
        if (data["exit code"] === 0 && /^[a-z]+$/.test(value) && value !== reported) {
            reported = value;
            watch();
        } else {
            // Missing or outdated service binary: retry without spinning.
            reported = "none";
            retryTimer.start();
        }
    }

    Component {
        id: watchComponent
        PlasmaCore.DataSource {
            engine: "executable"
            onNewData: {
                disconnectSource(sourceName);
                destroy();
                root.handleStatus(data);
            }
        }
    }

    Timer {
        id: retryTimer
        interval: 1000
        onTriggered: root.watch()
    }

    Component.onCompleted: watch()

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
